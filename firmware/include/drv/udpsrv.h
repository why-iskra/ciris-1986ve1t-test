//
// Created by whyiskra on 2025-06-06.
//

#pragma once

#include <stddef.h>
#include <stdint.h>
#include "mod/eth.h"

struct drv_udpsrv_ip {
    uint8_t octet1;
    uint8_t octet2;
    uint8_t octet3;
    uint8_t octet4;
} __attribute__((packed));

typedef void (*drv_udpsrv_handler_t)(const void *, size_t);

void drv_udpsrv_init(struct mod_eth_mac, drv_udpsrv_handler_t);

// int udpsrv_send(const void *, size_t);
