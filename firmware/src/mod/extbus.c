//
// Created by whyiskra on 2025-05-28.
//

#include "mod/extbus.h"
#include "mod/regs.h"
#include "utils/bits.h"

int extbus_setup(struct mod_extbus_cfg cfg) {
    struct extbus_regs regs = *get_extbus_regs();
    switch (cfg.mode) {
        case MOD_EXTBUS_OFF:
        case MOD_EXTBUS_ROM:
        case MOD_EXTBUS_RAM:
        case MOD_EXTBUS_NAND: set_bits(regs.control, 0, 0x7, cfg.mode); break;
        default: return -1;
    }

    set_bit(regs.control, 3, cfg.clock_polarity);
    set_bit(regs.control, 4, 0);
    set_bit(regs.control, 5, cfg.low8);
    set_bit(regs.control, 6, cfg.low16);
    set_bits(regs.control, 12, 0xf, cfg.wait_state & 0xf);

    regs.ram_cycles1 = 0x3FFF;
    regs.ram_cycles2 = 0x3FFF;
    regs.ram_cycles3 = 0x3FFF;
    regs.ram_cycles4 = 0x3FFF;

    *get_extbus_regs() = regs;

    return 0;
}
