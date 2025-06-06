//
// Created by whyiskra on 2025-05-27.
//

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define timer_default_cfg() ((struct mod_timer_cfg) { \
        .enable = false, \
        .initial_value = 0, \
        .trigger_value = 0, \
        .freq_div = 0, \
        .tim_div = 0, \
        .reverse = false, \
        .interrupt.zero = false, \
        .interrupt.trigger = false \
    })

typedef enum {
    MOD_TIMER_1,
    MOD_TIMER_2,
} mod_timer_t;

struct mod_timer_cfg {
    bool enable;
    uint32_t initial_value;
    uint32_t trigger_value;
    uint16_t freq_div;
    uint8_t tim_div;
    bool reverse;
    struct {
        bool zero;
        bool trigger;
    } interrupt;
};

int timer_setup(mod_timer_t, struct mod_timer_cfg);
int timer_control(mod_timer_t, bool);
uint32_t timer_value(mod_timer_t);
int timer_trigger(mod_timer_t, bool);
int timer_zero(mod_timer_t, bool);
