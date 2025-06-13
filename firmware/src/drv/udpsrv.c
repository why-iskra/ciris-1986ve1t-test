//
// Created by whyiskra on 2025-06-06.
//

#include "drv/udpsrv.h"
#include "config.h"
#include "drv/clock.h"
#include "drv/udpsrv/arp.h"
#include "drv/udpsrv/dhcp.h"
#include "drv/udpsrv/dynip.h"
#include "drv/udpsrv/frames.h"
#include "drv/udpsrv/utils.h"
#include "mod/clk.h"
#include "mod/nvic.h"
#include "mod/port.h"
#include "mod/timer.h"
#include <memory.h>

#define ORANGE_LED  MOD_PORT_B, 14
#define GREEN_LED   MOD_PORT_B, 15
#define CONNECT_LED MOD_PORT_D, 8
#define USER_LED    MOD_PORT_D, 9

typedef enum {
    STATE_INIT,
    STATE_READY,
    STATE_USER_ARP_REQUEST,
    STATE_USER_ARP_ANSWER,
    STATE_USER_SEND,
    STATE_USER_FAILED,
    STATE_DHCP_DISCOVER,
    STATE_DHCP_OFFER,
    STATE_DHCP_REQUEST,
    STATE_DHCP_ACK,
    STATE_DHCP_FINISH,
    STATE_DHCP_FAILED,
} state_t;

static drv_udpsrv_handler_t udp_handler;
static struct drv_udpsrv_ip default_ip;
static struct drv_udpsrv_ip default_mask;

static struct mod_eth_frame temp_frame;
static struct mod_eth_frame user_frame;

static state_t state;

static struct {
    bool has;
    size_t size;
    struct drv_udpsrv_ip dest_ip;
    uint16_t src_port;
    uint16_t dest_port;

    uint32_t timestamp;
    struct drv_udpsrv_ip src_ip;
    struct mod_eth_mac dest_mac;
} user;

static struct {
    int attempts;
    uint32_t xid;
    uint32_t timestamp;
    struct dhcp_offer offer;
} dhcp;

static void send_packet(struct mod_eth_frame *frame) {
    port_write(GREEN_LED, 1);
    eth_send(frame);
    port_write(GREEN_LED, 0);
}

// process

static void external_handle(void) {
    if (temp_frame.size < sizeof(struct udp_frame)) {
        return;
    }

    struct ethernet_frame *ethernet_frame =
        (struct ethernet_frame *) temp_frame.payload;
    struct ipv4_frame *ipv4_frame = (struct ipv4_frame *) temp_frame.payload;
    struct udp_frame *udp_frame = (struct udp_frame *) temp_frame.payload;

    if (!mac_eq(ethernet_frame->dest_mac, eth_mac())
        || ethernet_frame->ethertype != ETHERTYPE_IPV4) {
        return;
    }

    if (!frame_valid_ipv4(&temp_frame, IPV4_PROTOCOL_UDP)) {
        return;
    }

    struct drv_udpsrv_ip mask = dynip_mask();

    struct drv_udpsrv_ip ip = dynip_get();
    struct drv_udpsrv_ip masked_ip = ip_mask(ip, mask);

    struct drv_udpsrv_ip dest_ip = ipv4_frame->dest_ip;
    struct drv_udpsrv_ip masked_dest_ip = ip_mask(dest_ip, mask);

    if (!ip_eq(dest_ip, ip) || !ip_eq(masked_dest_ip, masked_ip)) {
        return;
    }

    if (!frame_valid_udp(&temp_frame, false, 0, false, 0)) {
        return;
    }

    frame_normalize_udp(&temp_frame);

    struct udp_info info = {
        .src_ip = ipv4_frame->src_ip,
        .dest_ip = ipv4_frame->dest_ip,
        .mask = mask,
        .src_port = udp_frame->src_port,
        .dest_port = udp_frame->dest_port,
    };

    udp_handler(
        temp_frame.payload + sizeof(struct udp_frame),
        udp_frame->length - UDP_FRAME_SIZE,
        info
    );
}

static void handle(void) {
    switch (state) {
        case STATE_USER_ARP_ANSWER: {
            if (arp_answer(&temp_frame, user.src_ip, &user.dest_mac)) {
                state = STATE_USER_SEND;
            }
            break;
        }
        case STATE_DHCP_OFFER: {
            if (dhcp_offer(&temp_frame, dhcp.xid, &dhcp.offer)) {
                state = STATE_DHCP_REQUEST;
            }
            return;
        }
        case STATE_DHCP_ACK: {
            if (dhcp_ack(&temp_frame, dhcp.xid, &dhcp.offer)) {
                state = STATE_DHCP_FINISH;
            }
            return;
        }
        case STATE_INIT:
        case STATE_DHCP_DISCOVER:
        case STATE_DHCP_REQUEST:
        case STATE_DHCP_FINISH:
        case STATE_DHCP_FAILED: return;
        default: break;
    }

    if (!dynip_has()) {
        return;
    }

    if (arp_external_request(&temp_frame, dynip_get())) {
        send_packet(&temp_frame);
        return;
    }

    if (udp_handler != NULL) {
        external_handle();
    }
}

static void receive(void) {
    if (!eth_receive(&temp_frame)) {
        port_write(ORANGE_LED, 1);
        handle();
        port_write(ORANGE_LED, 0);
    }
}

static void show_status(void) {
    port_write(CONNECT_LED, dynip_has());
    port_write(USER_LED, user.has);
}

static void process_state_ready(void) {
    if (!dynip_has()) {
        dhcp.attempts = 0;
        state = STATE_DHCP_DISCOVER;
    }

    if (user.has) {
        user.src_ip = dynip_get();
        state = STATE_USER_ARP_REQUEST;
    }
}

static void process_state_user_arp_request(void) {
    if (arp_table_get_ip(user.dest_ip, &user.dest_mac)) {
        state = STATE_USER_SEND;
        return;
    }

    arp_request(&temp_frame, user.src_ip, user.dest_ip);
    send_packet(&temp_frame);

    user.timestamp = clock_millis();
    state = STATE_USER_ARP_ANSWER;
}

static void process_state_user_arp_answer(void) {
    if (clock_millis() - user.timestamp > USER_TIMEOUT_MS) {
        state = STATE_USER_FAILED;
    }
}

static void process_state_user_send(void) {
    frame_setup_ethernet(&user_frame, user.dest_mac, ETHERTYPE_IPV4);

    frame_setup_ipv4(&user_frame, IPV4_PROTOCOL_UDP, user.src_ip, user.dest_ip);

    frame_setup_udp(&user_frame, user.src_port, user.dest_port, user.size);

    user_frame.size = sizeof(struct udp_frame) + user.size;

    send_packet(&user_frame);

    state = STATE_READY;
    user.has = false;
}

static void process_state_user_failed(void) {
    state = STATE_READY;
    user.has = false;
}

static void process_state_dhcp_discover(void) {
    dhcp.xid = clock_millis();

    dhcp_discover(&temp_frame, dhcp.xid, default_ip);
    send_packet(&temp_frame);

    dhcp.timestamp = clock_millis();
    state = STATE_DHCP_OFFER;
}

static void process_state_dhcp_wait(void) {
    if (clock_millis() - dhcp.timestamp > DHCP_TIMEOUT_MS) {
        state = STATE_DHCP_FAILED;
    }
}

static void process_state_dhcp_request(void) {
    dhcp_request(&temp_frame, dhcp.xid, dhcp.offer);
    send_packet(&temp_frame);

    dhcp.timestamp = clock_millis();
    state = STATE_DHCP_ACK;
}

static void process_state_dhcp_finish(void) {
    dynip_set(dhcp.offer.ip, dhcp.offer.mask, dhcp.offer.rent_time);
    state = STATE_READY;
}

static void process_state_dhcp_failed(void) {
    dhcp.attempts ++;
    if (dhcp.attempts > DHCP_RETRY_ATTEMPTS) {
        dynip_set(default_ip, default_mask, STATIC_IP_RENT_MS);
        state = STATE_READY;
    } else {
        state = STATE_DHCP_DISCOVER;
    }
}

static void process(void) {
    arp_table_validate();
    dynip_validate();

    show_status();

    receive();

    switch (state) {
        case STATE_INIT: return;
        case STATE_READY: process_state_ready(); return;
        case STATE_USER_ARP_REQUEST: process_state_user_arp_request(); return;
        case STATE_USER_ARP_ANSWER: process_state_user_arp_answer(); return;
        case STATE_USER_SEND: process_state_user_send(); return;
        case STATE_USER_FAILED: process_state_user_failed(); return;
        case STATE_DHCP_DISCOVER: process_state_dhcp_discover(); return;
        case STATE_DHCP_OFFER: process_state_dhcp_wait(); return;
        case STATE_DHCP_REQUEST: process_state_dhcp_request(); return;
        case STATE_DHCP_ACK: process_state_dhcp_wait(); return;
        case STATE_DHCP_FINISH: process_state_dhcp_finish(); return;
        case STATE_DHCP_FAILED: process_state_dhcp_failed(); return;
    }

    state = STATE_READY;
}

void __isr_timer1(void);
void __isr_timer1(void) {
    if (timer_trigger(MOD_TIMER_1, true)) {
        process();
    }
}

// api

void drv_udpsrv_init(
    struct mod_eth_mac mac,
    struct drv_udpsrv_ip ip,
    struct drv_udpsrv_ip mask,
    drv_udpsrv_handler_t handler
) {
    dynip_init();
    arp_init();

    state = STATE_INIT;
    user.has = false;
    udp_handler = handler;
    default_ip = ip;
    default_mask = mask;

    {
        clk_en_peripheral(MOD_CLK_PERIPHERAL_TIMER1, true);

        struct mod_timer_cfg timer_cfg = timer_default_cfg();
        timer_cfg.enable = true;
        timer_cfg.trigger_value = 8000;
        timer_cfg.interrupt.trigger = true;

        nvic_irq_en(MOD_NVIC_IRQ_TIMER1, false);
        timer_setup(MOD_TIMER_1, timer_cfg);
        timer_control(MOD_TIMER_1, true);
    }

    {
        clk_en_peripheral(MOD_CLK_PERIPHERAL_PORTB, true);
        clk_en_peripheral(MOD_CLK_PERIPHERAL_PORTD, true);

        struct mod_port_cfg cfg = port_default_cfg();
        cfg.func = MOD_PORT_FUNC_PORT;
        cfg.mode = MOD_PORT_MODE_DIGITAL;
        cfg.dir = MOD_PORT_DIR_OUT;
        cfg.speed = MOD_PORT_SPEED_FAST;

        port_setup(ORANGE_LED, cfg);
        port_setup(GREEN_LED, cfg);
        port_setup(CONNECT_LED, cfg);
        port_setup(USER_LED, cfg);
    }

    eth_setup(mac);
    nvic_irq_en(MOD_NVIC_IRQ_ETHERNET, true);
    nvic_irq_en(MOD_NVIC_IRQ_TIMER1, true);

    clock_delay_ms(3000);

    state = STATE_READY;
}

int udpsrv_send(
    struct drv_udpsrv_ip ip,
    uint16_t src_port,
    uint16_t dest_port,
    const void *data,
    size_t size
) {
    if (user.has) {
        return -1;
    }

    if (size > sizeof(user_frame.payload) - sizeof(struct udp_frame)) {
        return -1;
    }

    memcpy(user_frame.payload + sizeof(struct udp_frame), data, size);

    user.size = size;
    user.dest_ip = ip;
    user.src_port = src_port;
    user.dest_port = dest_port;

    user.has = true;

    return 0;
}
