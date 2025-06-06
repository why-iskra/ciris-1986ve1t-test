//
// Created by whyiskra on 2025-06-06.
//

#include "drv/udpsrv.h"
#include "mod/clk.h"
#include "mod/nvic.h"
#include "mod/port.h"
#include <memory.h>
#include <stdint.h>

#define ORANGE_LED MOD_PORT_B, 14
#define GREEN_LED  MOD_PORT_B, 15

#define BROADCAST_MAC ((struct mod_eth_mac) { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff })
#define BROADCAST_IP  ((struct drv_udpsrv_ip) { 0xff, 0xff, 0xff, 0xff })

typedef enum {
    ETHERTYPE_IPV4 = 0x0008,
    ETHERTYPE_ARP = 0x0608,
} ethertype_t;

typedef enum {
    ARP_OPCODE_REQUEST = 0x0100,
    ARP_OPCODE_ANSWER = 0x0200,
} arp_opcode_t;

typedef enum {
    ARP_HARDWARE_TYPE_ETHERNET = 0x0100,
} arp_hardware_type_t;

typedef enum {
    ARP_PROTOCOL_TYPE_IPV4 = 0x0008,
} arp_protocol_type_t;

struct ethernet_frame {
    struct mod_eth_mac dest_mac;
    struct mod_eth_mac src_mac;
    uint16_t ethertype;
} __attribute__((packed));

struct arp_frame {
    struct ethernet_frame ethernet;
    uint16_t hardware_type;
    uint16_t protocol_type;
    uint8_t hardware_size;
    uint8_t protocol_size;
    uint16_t opcode;
    struct mod_eth_mac sender_mac;
    struct drv_udpsrv_ip sender_ip;
    struct mod_eth_mac target_mac;
    struct drv_udpsrv_ip target_ip;
} __attribute__((packed));

struct ipv4_frame {
    struct ethernet_frame ethernet;
    uint8_t version : 4;
    uint8_t ihl     : 4;
    uint8_t dscp    : 6;
    uint8_t ecn     : 2;
    uint16_t length;
    uint16_t id;
    uint8_t flags            : 3;
    uint16_t fragment_offset : 13;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t header_checksum;
    struct drv_udpsrv_ip src_ip;
    struct drv_udpsrv_ip dest_ip;
} __attribute__((packed));

static drv_udpsrv_handler_t udp_handler = NULL;

static struct mod_eth_frame frame;

static struct {
    struct {
        bool has;
        struct drv_udpsrv_ip value;
    } ip;
} state = {
    .ip.has = false,
    .ip.value = { 0, 0, 0, 0 }
};

static void send_arp(
    arp_opcode_t,
    struct mod_eth_mac,
    struct mod_eth_mac,
    struct drv_udpsrv_ip,
    struct mod_eth_mac,
    struct drv_udpsrv_ip
);

static inline bool mac_eq(struct mod_eth_mac a, struct mod_eth_mac b) {
    return memcmp(&a, &b, sizeof(struct mod_eth_mac)) == 0;
}

static inline bool ip_eq(struct drv_udpsrv_ip a, struct drv_udpsrv_ip b) {
    return memcmp(&a, &b, sizeof(struct drv_udpsrv_ip)) == 0;
}

static void handle_arp(void) {
    if (frame.received < sizeof(struct arp_frame)) {
        return;
    }

    struct arp_frame *arp_frame = (struct arp_frame *) frame.payload;

    // is ethernet and ipv4?
    if (arp_frame->hardware_type != ARP_HARDWARE_TYPE_ETHERNET
        || arp_frame->protocol_type != ARP_PROTOCOL_TYPE_IPV4) {
        return;
    }

    // validate mac and ip length
    if (arp_frame->hardware_size != sizeof(struct mod_eth_mac)
        || arp_frame->protocol_size != sizeof(struct drv_udpsrv_ip)) {
        return;
    }

    if (arp_frame->opcode != ARP_OPCODE_REQUEST) {
        return;
    }

    if (!state.ip.has) {
        return;
    }

    if (ip_eq(state.ip.value, arp_frame->target_ip)) {
        send_arp(
            ARP_OPCODE_ANSWER,
            arp_frame->target_mac,
            eth_mac(),
            state.ip.value,
            arp_frame->sender_mac,
            arp_frame->sender_ip
        );
    }
}

static bool ethernet_check_mac(void) {
    struct ethernet_frame *ethernet = (struct ethernet_frame *) frame.payload;
    return mac_eq(ethernet->dest_mac, eth_mac())
           || mac_eq(ethernet->dest_mac, BROADCAST_MAC);
}

static void handle_ethernet(void) {
    if (frame.received < sizeof(struct ethernet_frame)) {
        return;
    }

    if (!ethernet_check_mac()) {
        return;
    }

    struct ethernet_frame *ethernet = (struct ethernet_frame *) frame.payload;
    switch ((ethertype_t) ethernet->ethertype) {
        case ETHERTYPE_IPV4: {
            break;
        }
        case ETHERTYPE_ARP: {
            handle_arp();
            break;
        }
        default: return;
    }
}

static void handle_frame(void) {
    port_write(ORANGE_LED, 1);

    if (eth_receive(&frame)) {
        port_write(ORANGE_LED, 0);
        return;
    }

    handle_ethernet();
    port_write(ORANGE_LED, 0);
}

void drv_udpsrv_init(struct mod_eth_mac mac, drv_udpsrv_handler_t handler) {
    struct mod_port_cfg cfg = port_default_cfg();
    cfg.func = MOD_PORT_FUNC_PORT;
    cfg.mode = MOD_PORT_MODE_DIGITAL;
    cfg.dir = MOD_PORT_DIR_OUT;
    cfg.speed = MOD_PORT_SPEED_FAST;

    clk_en_peripheral(MOD_CLK_PERIPHERAL_PORTB, true);
    port_setup(ORANGE_LED, cfg);
    port_setup(GREEN_LED, cfg);

    eth_setup(mac);
    eth_set_handler(&handle_frame);

    udp_handler = handler;

    nvic_irq_en(MOD_NVIC_IRQ_ETHERNET, true);
}

static void send_packet(const void *data, size_t size) {
    port_write(GREEN_LED, 1);
    eth_send(data, size);
    port_write(GREEN_LED, 0);
}

static void send_arp(
    arp_opcode_t opcode,
    struct mod_eth_mac ethernet_mac,
    struct mod_eth_mac source_mac,
    struct drv_udpsrv_ip source_ip,
    struct mod_eth_mac target_mac,
    struct drv_udpsrv_ip target_ip
) {
    struct arp_frame data = {
        .ethernet.dest_mac = ethernet_mac,
        .ethernet.src_mac = eth_mac(),
        .ethernet.ethertype = ETHERTYPE_ARP,
        .hardware_type = ARP_HARDWARE_TYPE_ETHERNET,
        .protocol_type = ARP_PROTOCOL_TYPE_IPV4,
        .hardware_size = sizeof(struct mod_eth_mac),
        .protocol_size = sizeof(struct drv_udpsrv_ip),
        .opcode = (uint16_t) opcode,
        .sender_mac = source_mac,
        .sender_ip = source_ip,
        .target_mac = target_mac,
        .target_ip = target_ip,
    };

    send_packet(&data, sizeof(data));
}
