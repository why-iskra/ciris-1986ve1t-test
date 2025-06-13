//
// Created by whyiskra on 2025-06-07.
//

#pragma once

#include "drv/udpsrv.h"
#include <stdbool.h>

#define IPV4_TTL (64)

#define ETHERNET_FRAME_SIZE (sizeof(struct ethernet_frame))
#define ARP_FRAME_SIZE      (sizeof(struct arp_frame) - sizeof(struct ethernet_frame))
#define IPV4_FRAME_SIZE     (sizeof(struct ipv4_frame) - sizeof(struct ethernet_frame))
#define UDP_FRAME_SIZE      (sizeof(struct udp_frame) - sizeof(struct ipv4_frame))
#define BOOTP_FRAME_SIZE    (sizeof(struct bootp_frame) - sizeof(struct udp_frame))

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

typedef enum {
    IPV4_PROTOCOL_ICMP = 0x01,
    IPV4_PROTOCOL_UDP = 0x11,
} ipv4_protocol_t;

typedef enum {
    BOOTP_OP_REQUEST = 0x01,
    BOOTP_OP_REPLY = 0x02,
} bootp_op_t;

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
    uint8_t version;
    uint8_t dos;
    uint16_t length;
    uint16_t id;
    uint16_t fragment;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    struct drv_udpsrv_ip src_ip;
    struct drv_udpsrv_ip dest_ip;
} __attribute__((packed));

struct udp_frame {
    struct ipv4_frame ipv4;
    uint16_t src_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed));

struct bootp_frame {
    struct udp_frame udp;
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    struct drv_udpsrv_ip ciaddr;
    struct drv_udpsrv_ip yiaddr;
    struct drv_udpsrv_ip siaddr;
    struct drv_udpsrv_ip giaddr;
    struct {
        struct mod_eth_mac mac;
        uint8_t _reserved[16 - sizeof(struct mod_eth_mac)];
    } __attribute__((packed)) chaddr;
    uint8_t sname[64];
    uint8_t file[128];
} __attribute__((packed));

void frame_setup_ethernet(
    struct mod_eth_frame *,
    struct mod_eth_mac,
    ethertype_t
);

void frame_setup_arp(
    struct mod_eth_frame *,
    arp_opcode_t,
    struct mod_eth_mac,
    struct drv_udpsrv_ip,
    struct mod_eth_mac,
    struct drv_udpsrv_ip
);

bool frame_valid_arp(struct mod_eth_frame *, arp_opcode_t);

void frame_setup_ipv4(
    struct mod_eth_frame *,
    ipv4_protocol_t,
    struct drv_udpsrv_ip,
    struct drv_udpsrv_ip
);

bool frame_valid_ipv4(struct mod_eth_frame *, ipv4_protocol_t);

void frame_setup_udp(struct mod_eth_frame *, uint16_t, uint16_t, size_t);

bool frame_valid_udp(struct mod_eth_frame *, bool, uint16_t, bool, uint16_t);

void frame_setup_bootp(
    struct mod_eth_frame *,
    bootp_op_t,
    uint32_t,
    struct drv_udpsrv_ip,
    struct drv_udpsrv_ip,
    struct drv_udpsrv_ip,
    struct drv_udpsrv_ip,
    struct mod_eth_mac
);

bool frame_valid_bootp(struct mod_eth_frame *);
