//
// Created by whyiskra on 2025-05-28.
//

#include "mod/reset.h"
#include "mod/regs.h"

void software_reset(void) {
    *get_aircr_reg() = 0x05FA0004;
}
