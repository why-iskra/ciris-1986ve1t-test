//
// Created by whyiskra on 2025-05-28.
//

#include "mod/watchdog.h"
#include "mod/regs.h"

#define START_KEY  (0xCCCC)
#define ACCESS_KEY (0x5555)
#define NOTIFY_KEY (0xAAAA)

int iwdt_setup(mod_iwdg_freq_div_t div, uint16_t reload) {
    switch (div) {
        case MOD_IWDG_FREQ_DIV_4:
        case MOD_IWDG_FREQ_DIV_8:
        case MOD_IWDG_FREQ_DIV_16:
        case MOD_IWDG_FREQ_DIV_32:
        case MOD_IWDG_FREQ_DIV_64:
        case MOD_IWDG_FREQ_DIV_128:
        case MOD_IWDG_FREQ_DIV_256: break;
        default: return -1;
    }

    struct iwdt_regs *regs = get_iwdt_regs();
    regs->kr = ACCESS_KEY;
    while (regs->sr != 0) { }
    regs->pr = div & 0x7;
    regs->prl = reload & 0xFFF;

    regs->kr = START_KEY;
    return 0;
}

void iwdt_notify(void) {
    get_iwdt_regs()->kr = NOTIFY_KEY;
}
