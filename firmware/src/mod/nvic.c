//
// Created by whyiskra on 2025-05-28.
//

#include "mod/nvic.h"
#include "mod/regs.h"
#include "utils/bits.h"

static int validate_irq(mod_nvic_irq_t irq) {
    switch (irq) {
        case MOD_NVIC_IRQ_MIL_STD_1553B2:
        case MOD_NVIC_IRQ_MIL_STD_1553B1:
        case MOD_NVIC_IRQ_USB:
        case MOD_NVIC_IRQ_CAN1:
        case MOD_NVIC_IRQ_CAN2:
        case MOD_NVIC_IRQ_DMA:
        case MOD_NVIC_IRQ_UART1:
        case MOD_NVIC_IRQ_UART2:
        case MOD_NVIC_IRQ_SSP1:
        case MOD_NVIC_IRQ_BUSY:
        case MOD_NVIC_IRQ_ARINC429R:
        case MOD_NVIC_IRQ_POWER:
        case MOD_NVIC_IRQ_WWDG:
        case MOD_NVIC_IRQ_TIMER4:
        case MOD_NVIC_IRQ_TIMER1:
        case MOD_NVIC_IRQ_TIMER2:
        case MOD_NVIC_IRQ_TIMER3:
        case MOD_NVIC_IRQ_ADC:
        case MOD_NVIC_IRQ_ETHERNET:
        case MOD_NVIC_IRQ_SSP3:
        case MOD_NVIC_IRQ_SSP2:
        case MOD_NVIC_IRQ_ARINC42T1:
        case MOD_NVIC_IRQ_ARINC42T2:
        case MOD_NVIC_IRQ_ARINC42T3:
        case MOD_NVIC_IRQ_ARINC42T4:
        case MOD_NVIC_IRQ_BACKUP:
        case MOD_NVIC_IRQ_EXT_INT1:
        case MOD_NVIC_IRQ_EXT_INT2:
        case MOD_NVIC_IRQ_EXT_INT3:
        case MOD_NVIC_IRQ_EXT_INT4: return 0;
    }

    return -1;
}

int nvic_irq_en(mod_nvic_irq_t irq, bool en) {
    if (validate_irq(irq)) {
        return -1;
    }

    if (en) {
        set_bit(*get_nvic_reg(ISER), irq, 1);
    } else {
        set_bit(*get_nvic_reg(ICER), irq, 1);
    }

    return 0;
}

int nvic_irq_is_en(mod_nvic_irq_t irq) {
    if (validate_irq(irq)) {
        return -1;
    }

    return get_bit(*get_nvic_reg(ISER), irq);
}

int nvic_irq_set_pend(mod_nvic_irq_t irq, bool en) {
    if (validate_irq(irq)) {
        return -1;
    }

    if (en) {
        set_bit(*get_nvic_reg(ISPR), irq, 1);
    } else {
        set_bit(*get_nvic_reg(ICPR), irq, 1);
    }

    return 0;
}

int nvic_irq_is_pend(mod_nvic_irq_t irq) {
    if (validate_irq(irq)) {
        return -1;
    }

    return get_bit(*get_nvic_reg(ISPR), irq);
}
