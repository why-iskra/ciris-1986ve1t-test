//
// Created by whyiskra on 2025-05-27.
//

#include "mod/scb/systick.h"
#include "mod/regs.h"
#include "utils/bits.h"

void scb_systick_setup(struct mod_scb_systick_cfg cfg) {
    struct scb_systick_regs *regs = get_scb_systick_regs();

    if (cfg.enable) {
        regs->systick_load = cfg.load & 0x00FFFFFF;
        set_bit(regs->systick_ctrl, 1, cfg.interrupt);
        set_bit(regs->systick_ctrl, 0, cfg.enable);
    } else {
        set_bit(regs->systick_ctrl, 0, 0);
        set_bit(regs->systick_ctrl, 1, 0);
    }
}

bool scb_systick_count(void) {
    return get_bit(get_scb_systick_regs()->systick_ctrl, 16) == 1;
}

void scb_systick_reset(void) {
    get_scb_systick_regs()->systick_val = 0;
}

uint32_t scb_systick_value(void) {
    uint32_t value = get_scb_systick_regs()->systick_val;
    return value & 0x00FFFFFF;
}
