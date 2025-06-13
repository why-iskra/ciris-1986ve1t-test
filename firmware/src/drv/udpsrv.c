//
// Created by whyiskra on 2025-06-06.
//

#include "drv/udpsrv.h"
#include "config.h"
#include "drv/clock.h"
#include "drv/udpsrv/arptable.h"
#include "drv/udpsrv/dhcp.h"
#include "drv/udpsrv/dynip.h"
#include "drv/udpsrv/frames.h"
#include "drv/udpsrv/utils.h"
#include "mod/clk.h"
#include "mod/nvic.h"
#include "mod/port.h"
#include "mod/timer.h"

#define ORANGE_LED  MOD_PORT_B, 14
#define GREEN_LED   MOD_PORT_B, 15
#define CONNECT_LED MOD_PORT_D, 8

typedef enum {
    STATE_INIT,
    STATE_READY,
    STATE_DHCP_DISCOVER,
    STATE_DHCP_OFFER,
    STATE_DHCP_REQUEST,
    STATE_DHCP_ACK,
    STATE_DHCP_FINISH,
    STATE_DHCP_FAILED,
} state_t;

static drv_udpsrv_handler_t udp_handler;
static struct drv_udpsrv_ip default_ip;

static struct mod_eth_frame recevice_frame;
static struct mod_eth_frame send_frame;
static struct mod_eth_frame user_frame;

static state_t state;

static struct {
    uint32_t xid;
    uint32_t timestamp;
    struct dhcp_offer offer;
} dhcp;

// send

static void send_packet(struct mod_eth_frame *frame) {
    port_write(GREEN_LED, 1);
    eth_send(frame);
    port_write(GREEN_LED, 0);
}

// handlers

static void handle_arp_request(void) {
    struct arp_frame *arp_frame = (struct arp_frame *) recevice_frame.payload;

    struct drv_udpsrv_ip ip = dynip_get();
    if (!ip_eq(ip, arp_frame->target_ip)) {
        return;
    }

    frame_setup_ethernet(&send_frame, arp_frame->sender_mac, ETHERTYPE_ARP);

    frame_setup_arp(
        &send_frame,
        ARP_OPCODE_ANSWER,
        eth_mac(),
        ip,
        arp_frame->sender_mac,
        arp_frame->sender_ip
    );

    send_frame.size = sizeof(struct arp_frame);

    send_packet(&send_frame);
}

static void handle_arp_answer(void) {
    struct arp_frame *arp_frame = (struct arp_frame *) recevice_frame.payload;
    if (!mac_eq(eth_mac(), arp_frame->target_mac)) {
        return;
    }

    if (!ip_eq(dynip_get(), arp_frame->target_ip)) {
        return;
    }

    arp_table_set_ip(arp_frame->sender_ip, arp_frame->sender_mac);
}

static void handle_arp(void) {
    if (recevice_frame.size < sizeof(struct arp_frame)) {
        return;
    }

    struct arp_frame *arp_frame = (struct arp_frame *) recevice_frame.payload;

    if (arp_frame->hardware_type != ARP_HARDWARE_TYPE_ETHERNET
        || arp_frame->protocol_type != ARP_PROTOCOL_TYPE_IPV4) {
        return;
    }

    if (arp_frame->hardware_size != sizeof(struct mod_eth_mac)
        || arp_frame->protocol_size != sizeof(struct drv_udpsrv_ip)) {
        return;
    }

    if (arp_frame->opcode == ARP_OPCODE_REQUEST) {
        handle_arp_request();
    } else if (arp_frame->opcode == ARP_OPCODE_ANSWER) {
        handle_arp_answer();
    }
}

static void handle_ipv4(void) {
    if (recevice_frame.size < sizeof(struct ipv4_frame)) {
        return;
    }

    struct ipv4_frame *ipv4_frame =
        (struct ipv4_frame *) recevice_frame.payload;
}

static void handle_ethernet(void) {
    switch (state) {
        case STATE_INIT: return;
        case STATE_DHCP_DISCOVER: return;
        case STATE_DHCP_OFFER: {
            if (dhcp_offer(&recevice_frame, dhcp.xid, &dhcp.offer)) {
                state = STATE_DHCP_REQUEST;
            }
            return;
        }
        case STATE_DHCP_REQUEST: return;
        case STATE_DHCP_ACK: {
            if (dhcp_ack(&recevice_frame, dhcp.xid, &dhcp.offer)) {
                state = STATE_DHCP_FINISH;
            }
            return;
        }
        case STATE_DHCP_FINISH: return;
        case STATE_DHCP_FAILED: return;
        default: break;
    }

    if (!dynip_has()) {
        return;
    }

    if (recevice_frame.size < sizeof(struct ethernet_frame)) {
        return;
    }

    struct ethernet_frame *ethernet =
        (struct ethernet_frame *) recevice_frame.payload;
    if (!mac_eq(ethernet->dest_mac, eth_mac())
        && !mac_eq(ethernet->dest_mac, BROADCAST_MAC)) {
        return;
    }

    switch ((ethertype_t) ethernet->ethertype) {
        case ETHERTYPE_IPV4: {
            handle_ipv4();
            break;
        }
        case ETHERTYPE_ARP: {
            handle_arp();
            break;
        }
        default: return;
    }
}

// process

static void receive(void) {
    if (!eth_receive(&recevice_frame)) {
        port_write(ORANGE_LED, 1);
        handle_ethernet();
        port_write(ORANGE_LED, 0);
    }
}

static void show_status(void) {
    port_write(CONNECT_LED, dynip_has());
}

static void process(void) {
    arp_table_validate();
    dynip_validate();

    show_status();

    receive();

    switch (state) {
        case STATE_INIT: break;
        case STATE_READY: {
            if (!dynip_has()) {
                state = STATE_DHCP_DISCOVER;
            }
            break;
        }
        case STATE_DHCP_DISCOVER: {
            dhcp.xid = clock_millis();

            dhcp_discover(&send_frame, dhcp.xid, default_ip);
            send_packet(&send_frame);
            dhcp.timestamp = clock_millis();
            state = STATE_DHCP_OFFER;
            break;
        }
        case STATE_DHCP_OFFER: {
            if (clock_millis() - dhcp.timestamp > DHCP_TIMEOUT_MS) {
                state = STATE_DHCP_FAILED;
            }
            break;
        }
        case STATE_DHCP_REQUEST: {
            dhcp_request(&send_frame, dhcp.xid, dhcp.offer);
            send_packet(&send_frame);
            dhcp.timestamp = clock_millis();
            state = STATE_DHCP_ACK;
            break;
        }
        case STATE_DHCP_ACK: {
            if (clock_millis() - dhcp.timestamp > DHCP_TIMEOUT_MS) {
                state = STATE_DHCP_FAILED;
            }
            break;
        }
        case STATE_DHCP_FINISH: {
            dynip_set(dhcp.offer.ip, dhcp.offer.rent_time);
            state = STATE_READY;
            break;
        }
        case STATE_DHCP_FAILED: {
            dynip_set(default_ip, STATIC_IP_RENT_MS);
            state = STATE_READY;
            break;
        }
    }
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
    drv_udpsrv_handler_t handler
) {
    dynip_init();
    arp_table_init();

    state = STATE_INIT;
    udp_handler = handler;
    default_ip = ip;

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
    }

    eth_setup(mac);
    nvic_irq_en(MOD_NVIC_IRQ_ETHERNET, true);
    nvic_irq_en(MOD_NVIC_IRQ_TIMER1, true);

    clock_delay_ms(3000);

    state = STATE_READY;
}

int udpsrv_send(void) {
    // if (state != STATE_READY) {
    //     return 1;
    // }

    // struct drv_udpsrv_ip dest_ip = { 192, 168, 1, 1 };
    // frame_setup_ipv4(&user_frame, 0, IPV4_PROTOCOL_UDP, ip.value, dest_ip);
    // frame_setup_udp(&user_frame, 25565, 25565, 0);
    //
    // user_frame.size = sizeof(struct udp_frame);

    // struct drv_udpsrv_ip ip = { 192, 168, 0, 97 };
    // dhcp_discover(&user_frame, 0x1234, ip);
    // send_packet(&user_frame);

    return 0;
}
