//
// Created by whyiskra on 2025-05-28.
//

#include "drv/clock.h"
#include "config.h"
#include "mod/clk.h"
#include "mod/nvic.h"
#include "mod/timer.h"
#include <stdbool.h>

static volatile bool delay_end = false;

void __isr_timer1(void);
void __isr_timer1(void) {
    if (timer_trigger(MOD_TIMER_1, true)) {
        timer_control(MOD_TIMER_1, false);
        delay_end = true;
    }
}

void drv_clock_init(void) {
    clk_en_peripheral(MOD_CLK_PERIPHERAL_TIMER1, true);
    nvic_irq_en(MOD_NVIC_IRQ_TIMER1, true);
}

int clock_delay(uint32_t value, drv_clock_unit_t unit) {
    struct mod_timer_cfg timer_cfg = timer_default_cfg();
    timer_cfg.enable = true;
    timer_cfg.interrupt.trigger = true;

    uint16_t cpuf_mul = clk_get_cpu_freq() / BASE_FREQ_HZ;
    switch (unit) {
        case DRV_CLOCK_UNIT_MS: {
            timer_cfg.freq_div = 1000 * (cpuf_mul) - 1;
            break;
        }
        case DRV_CLOCK_UNIT_US: {
            timer_cfg.freq_div = cpuf_mul - 1;
            break;
        }
        default: return -1;
    }

    timer_cfg.trigger_value = (BASE_FREQ_HZ / 1000000) * value;

    delay_end = false;

    timer_setup(MOD_TIMER_1, timer_cfg);
    timer_control(MOD_TIMER_1, true);
    while (!delay_end) { }
    timer_control(MOD_TIMER_1, false);

    return 0;
}
