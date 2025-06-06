//
// Created by whyiskra on 2025-05-27.
//

#pragma once

#include <stdbool.h>
#include <stdint.h>

#define scb_systick_default_cfg() ((struct mod_scb_systick_cfg) { .enable = false, .interrupt = false, .load = 0 })

struct mod_scb_systick_cfg {
    bool enable;
    bool interrupt;
    uint32_t load;
};

void scb_systick_setup(struct mod_scb_systick_cfg);
bool scb_systick_count(void);
void scb_systick_reset(void);
uint32_t scb_systick_value(void);
