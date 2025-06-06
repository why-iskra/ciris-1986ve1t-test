//
// Created by whyiskra on 2025-05-28.
//

#pragma once

#include <stdbool.h>

typedef enum {
    MOD_NVIC_IRQ_MIL_STD_1553B2 = 0,
    MOD_NVIC_IRQ_MIL_STD_1553B1 = 1,
    MOD_NVIC_IRQ_USB = 2,
    MOD_NVIC_IRQ_CAN1 = 3,
    MOD_NVIC_IRQ_CAN2 = 4,
    MOD_NVIC_IRQ_DMA = 5,
    MOD_NVIC_IRQ_UART1 = 6,
    MOD_NVIC_IRQ_UART2 = 7,
    MOD_NVIC_IRQ_SSP1 = 8,
    MOD_NVIC_IRQ_BUSY = 9,
    MOD_NVIC_IRQ_ARINC429R = 10,
    MOD_NVIC_IRQ_POWER = 11,
    MOD_NVIC_IRQ_WWDG = 12,
    MOD_NVIC_IRQ_TIMER4 = 13,
    MOD_NVIC_IRQ_TIMER1 = 14,
    MOD_NVIC_IRQ_TIMER2 = 15,
    MOD_NVIC_IRQ_TIMER3 = 16,
    MOD_NVIC_IRQ_ADC = 17,
    MOD_NVIC_IRQ_ETHERNET = 18,
    MOD_NVIC_IRQ_SSP3 = 19,
    MOD_NVIC_IRQ_SSP2 = 20,
    MOD_NVIC_IRQ_ARINC42T1 = 21,
    MOD_NVIC_IRQ_ARINC42T2 = 22,
    MOD_NVIC_IRQ_ARINC42T3 = 23,
    MOD_NVIC_IRQ_ARINC42T4 = 24,
    MOD_NVIC_IRQ_BACKUP = 27,
    MOD_NVIC_IRQ_EXT_INT1 = 28,
    MOD_NVIC_IRQ_EXT_INT2 = 29,
    MOD_NVIC_IRQ_EXT_INT3 = 30,
    MOD_NVIC_IRQ_EXT_INT4 = 31,
} mod_nvic_irq_t;

int nvic_irq_en(mod_nvic_irq_t, bool);
int nvic_irq_is_en(mod_nvic_irq_t);
int nvic_irq_set_pend(mod_nvic_irq_t, bool);
int nvic_irq_is_pend(mod_nvic_irq_t);
