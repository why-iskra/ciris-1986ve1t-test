//
// Created by whyiskra on 2025-05-28.
//

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    MOD_EXTBUS_OFF = 0x0,
    MOD_EXTBUS_ROM = 0x1,
    MOD_EXTBUS_RAM = 0x2,
    MOD_EXTBUS_NAND = 0x4,
} mod_extbus_t;

struct mod_extbus_cfg {
    mod_extbus_t mode;
    bool low8;
    bool low16;
    bool clock_polarity;
    uint8_t wait_state;
};

#define extbus_default_cfg() ((struct mod_extbus_cfg) { .mode = MOD_EXTBUS_OFF, .low8 = 0, .low16 = 0, .clock_polarity = false, .wait_state = 0xF })

int extbus_setup(struct mod_extbus_cfg);
