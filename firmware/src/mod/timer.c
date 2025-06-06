//
// Created by whyiskra on 2025-05-27.
//

#include "mod/timer.h"
#include "mod/regs.h"
#include "utils/bits.h"
#include <memory.h>
#include <stddef.h>

static struct timer_regs *get_timer(mod_timer_t timer) {
    switch (timer) {
        case MOD_TIMER_1: return get_timer_regs(1);
        case MOD_TIMER_2: return get_timer_regs(2);
    }

    return NULL;
}

int timer_setup(mod_timer_t timer, struct mod_timer_cfg cfg) {
    struct timer_regs *regs = get_timer(timer);
    if (regs == NULL) {
        return -1;
    }

    regs->cntrl = 0;
    memset(regs, 0, sizeof(struct timer_regs));

    if (!cfg.enable) {
        return 0;
    }

    regs->cnt = cfg.initial_value;
    regs->arr = cfg.trigger_value;
    regs->psg = cfg.freq_div;

    if (cfg.interrupt.zero) {
        set_bit(regs->ie, 0, 1);
    }

    if (cfg.interrupt.trigger) {
        set_bit(regs->ie, 1, 1);
    }

    set_bit(regs->cntrl, 3, cfg.reverse);
    set_bit(regs->cntrl, 0, 1);

    return 0;
}

int timer_control(mod_timer_t timer, bool start) {
    struct rst_clk_regs *regs = get_rst_clk_regs();
    switch (timer) {
        case MOD_TIMER_1: {
            bool result = get_bit(regs->tim_clock, 24);
            set_bit(regs->tim_clock, 24, start);
            return result;
        }
        case MOD_TIMER_2: {
            bool result = get_bit(regs->tim_clock, 25);
            set_bit(regs->tim_clock, 25, start);
            return result;
        }
    }

    return -1;
}

uint32_t timer_value(mod_timer_t timer) {
    struct timer_regs *regs = get_timer(timer);
    if (regs == NULL) {
        return 0;
    }

    return regs->cnt;
}

int timer_trigger(mod_timer_t timer, bool reset) {
    struct timer_regs *regs = get_timer(timer);
    if (regs == NULL) {
        return -1;
    }

    int is_started = timer_control(timer, false);

    bool result = get_bit(regs->status, 1);
    if (reset) {
        set_bit(regs->status, 1, 0);
    }

    timer_control(timer, is_started);

    return result;
}

int timer_zero(mod_timer_t timer, bool reset) {
    struct timer_regs *regs = get_timer(timer);
    if (regs == NULL) {
        return -1;
    }

    int is_started = timer_control(timer, false);

    bool result = get_bit(regs->status, 0);
    if (reset) {
        set_bit(regs->status, 0, 0);
    }

    timer_control(timer, is_started);

    return result;
}
