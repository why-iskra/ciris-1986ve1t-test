//
// Created by whyiskra on 2025-05-28.
//

#pragma once

#include <stdint.h>

typedef enum {
    MOD_IWDG_FREQ_DIV_4 = 0,
    MOD_IWDG_FREQ_DIV_8 = 1,
    MOD_IWDG_FREQ_DIV_16 = 2,
    MOD_IWDG_FREQ_DIV_32 = 3,
    MOD_IWDG_FREQ_DIV_64 = 4,
    MOD_IWDG_FREQ_DIV_128 = 5,
    MOD_IWDG_FREQ_DIV_256 = 6,
} mod_iwdg_freq_div_t;

int iwdt_setup(mod_iwdg_freq_div_t, uint16_t);
void iwdt_notify(void);
