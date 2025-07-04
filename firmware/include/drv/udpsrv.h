//
// Created by whyiskra on 2025-06-06.
//

#pragma once

#include "mod/eth.h"
#include <stddef.h>
#include <stdint.h>

#define FILL_MAC(x) ((struct mod_eth_mac) { (x), (x), (x), (x), (x), (x) })
#define FILL_IP(x)  ((struct drv_udpsrv_ip) { (x), (x), (x), (x) })

#define ZERO_MAC FILL_MAC(0)
#define ZERO_IP  FILL_IP(0)

#define BROADCAST_MAC FILL_MAC(0xff)
#define BROADCAST_IP  FILL_IP(0xff)

struct drv_udpsrv_ip {
    uint8_t octet1;
    uint8_t octet2;
    uint8_t octet3;
    uint8_t octet4;
} __attribute__((packed));

struct udp_info {
    struct mod_eth_mac server_mac;
    struct drv_udpsrv_ip server_ip;
    struct drv_udpsrv_ip src_ip;
    struct drv_udpsrv_ip dest_ip;
    struct drv_udpsrv_ip mask;
    uint16_t src_port;
    uint16_t dest_port;
};

typedef void (*drv_udpsrv_handler_t)(const void *, size_t, struct udp_info);

void drv_udpsrv_init(
    struct mod_eth_mac,
    struct drv_udpsrv_ip,
    struct drv_udpsrv_ip,
    drv_udpsrv_handler_t
);

int udpsrv_send(struct drv_udpsrv_ip, uint16_t, uint16_t, const void *, size_t);
