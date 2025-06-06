//
// Created by whyiskra on 2025-05-30.
//

#pragma once

#include <stddef.h>
#include <stdint.h>

#define MOD_ETH_PAYLOAD_SIZE (3072)

struct mod_eth_mac {
    uint8_t value[6];
};

struct mod_eth_frame {
    size_t received;
    union {
        uint8_t payload[MOD_ETH_PAYLOAD_SIZE];
        uint32_t payload_word[MOD_ETH_PAYLOAD_SIZE / 4];
    };
};

void eth_setup(struct mod_eth_mac);
int read_packet(struct mod_eth_frame *);
int send_packet(const void *, size_t);
