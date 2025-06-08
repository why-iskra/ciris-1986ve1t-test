//
// Created by whyiskra on 2025-05-30.
//

#pragma once

#include <stddef.h>
#include <stdint.h>

#define MOD_ETH_PAYLOAD_SIZE (3072)

typedef void (*mod_eth_handler_t)(void);

struct mod_eth_mac {
    uint8_t octet1;
    uint8_t octet2;
    uint8_t octet3;
    uint8_t octet4;
    uint8_t octet5;
    uint8_t octet6;
} __attribute__((packed));

struct mod_eth_frame {
    size_t size;
    union {
        uint8_t payload[MOD_ETH_PAYLOAD_SIZE];
        uint32_t payload_word[MOD_ETH_PAYLOAD_SIZE / 4];
    };
};

void eth_setup(struct mod_eth_mac);
void eth_set_handler(mod_eth_handler_t);
struct mod_eth_mac eth_mac(void);
int eth_receive(struct mod_eth_frame *);
int eth_send(struct mod_eth_frame *);
