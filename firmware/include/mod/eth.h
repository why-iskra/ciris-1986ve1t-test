//
// Created by whyiskra on 2025-05-30.
//

#pragma once

#include <stddef.h>
#include <stdint.h>

struct mod_eth_mac {
    uint8_t value[6];
};

struct mod_eth_frame {
    uint16_t payload[288];
    uint16_t counter;
    uint16_t status;
};

void eth_setup(struct mod_eth_mac);
int send_packet(const void *, size_t);
// size_t read_packet(struct mod_eth_frame *);
