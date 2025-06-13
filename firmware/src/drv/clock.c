//
// Created by whyiskra on 2025-05-28.
//

#include "drv/clock.h"
#include "config.h"
#include "mod/clk.h"
#include "mod/scb/systick.h"
#include "mod/timer.h"
#include <stdbool.h>

static volatile bool systick_end;

void __isr_systick(void);
void __isr_systick(void) {
    systick_end = true;
    scb_systick_setup(scb_systick_default_cfg());
}

void drv_clock_init(void) {
    clk_en_peripheral(MOD_CLK_PERIPHERAL_TIMER2, true);
    struct mod_timer_cfg timer_cfg = timer_default_cfg();
    timer_cfg.enable = true;
    timer_cfg.freq_div = (uint16_t) ((clk_get_cpu_freq() / BASE_FREQ_HZ) * 125) - 1;
    timer_cfg.tim_div = 6;
    timer_cfg.trigger_value = UINT32_MAX;
    timer_setup(MOD_TIMER_2, timer_cfg);
    timer_control(MOD_TIMER_2, true);
}

void clock_delay_us(uint8_t value) {
    struct mod_scb_systick_cfg cfg = scb_systick_default_cfg();
    cfg.enable = true;
    cfg.interrupt = true;
    cfg.load = 8 * (clk_get_cpu_freq() / BASE_FREQ_HZ) * value;

    systick_end = false;
    scb_systick_setup(cfg);
    while (!systick_end) { }
}

void clock_delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms; i++) {
        clock_delay_us(250);
        clock_delay_us(250);
        clock_delay_us(250);
        clock_delay_us(250);
    }
}

uint32_t clock_millis(void) {
    return timer_value(MOD_TIMER_2);
}
