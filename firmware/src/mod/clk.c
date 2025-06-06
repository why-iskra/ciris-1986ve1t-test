//
// Created by whyiskra on 2025-05-24.
//

#include "mod/clk.h"
#include "config.h"
#include "mod/regs.h"
#include "utils/bits.h"

static uint32_t cpu_freq_multiplier = 1;

void clk_init(void) {
    struct rst_clk_regs *regs = get_rst_clk_regs();
    regs->per_clock = 0x8000010;
    regs->cpu_clock = 0x00000000;
    regs->pll_control = 0x00000000;
    regs->hs_control = 0x00000000;
    regs->usb_clock = 0x00000000;
    regs->adc_mco_clock = 0x00000000;
}

static void disable_pll(void) {
    struct rst_clk_regs *regs = get_rst_clk_regs();
    cpu_freq_multiplier = 1;

    set_bit(regs->cpu_clock, 2, 0); // CPU_C2 = CPU_C1

    set_bit(regs->pll_control, 2, 0);       // off pll
    set_bits(regs->pll_control, 8, 0xf, 0); // set x1
}

static void enable_pll(uint8_t mul) {
    struct rst_clk_regs *regs = get_rst_clk_regs();
    cpu_freq_multiplier = (mul & 0xf) + 1;

    set_bits(regs->pll_control, 8, 0xf, mul & 0xf); // set mul
    set_bit(regs->pll_control, 2, 1);               // en pll

    // restart pll
    set_bit(regs->pll_control, 3, 1);
    set_bit(regs->pll_control, 3, 0);
    while(!get_bit(regs->clock_status, 1)) {}

    set_bit(regs->cpu_clock, 2, 1); // CPU_C2 = PLL
    set_bits(regs->cpu_clock, 8, 0x3, 1); // HCLK = CPU_C3
}

int clk_set_cpu_freq(mod_clk_cpu_freq_t freq) {
    switch (freq) {
        case MOD_CLK_CPU_FREQ_8MHZ: {
            disable_pll();
            break;
        }
        case MOD_CLK_CPU_FREQ_16MHZ: {
            enable_pll(1);
            break;
        }
        case MOD_CLK_CPU_FREQ_32MHZ: {
            enable_pll(3);
            break;
        }
        case MOD_CLK_CPU_FREQ_64MHZ: {
            enable_pll(7);
            break;
        }
        case MOD_CLK_CPU_FREQ_96MHZ: {
            enable_pll(11);
            break;
        }
        default: return -1;
    }

    return 0;
}

uint32_t clk_get_cpu_freq(void) {
    return BASE_FREQ_HZ * cpu_freq_multiplier;
}

int clk_en_peripheral(mod_clk_peripheral_t clk_peripheral, bool value) {
    switch (clk_peripheral) {
        case MOD_CLK_PERIPHERAL_CAN1:
        case MOD_CLK_PERIPHERAL_CAN2:
        case MOD_CLK_PERIPHERAL_USB:
        case MOD_CLK_PERIPHERAL_EEPROM_CNTRL:
        case MOD_CLK_PERIPHERAL_RST_CLK:
        case MOD_CLK_PERIPHERAL_DMA:
        case MOD_CLK_PERIPHERAL_UART1:
        case MOD_CLK_PERIPHERAL_UART2:
        case MOD_CLK_PERIPHERAL_SPI1:
        case MOD_CLK_PERIPHERAL_MIL_STD_1553B1:
        case MOD_CLK_PERIPHERAL_MIL_STD_1553B2:
        case MOD_CLK_PERIPHERAL_POWER:
        case MOD_CLK_PERIPHERAL_WWDT:
        case MOD_CLK_PERIPHERAL_IWDT:
        case MOD_CLK_PERIPHERAL_TIMER1:
        case MOD_CLK_PERIPHERAL_TIMER2:
        case MOD_CLK_PERIPHERAL_TIMER3:
        case MOD_CLK_PERIPHERAL_ADC:
        case MOD_CLK_PERIPHERAL_DAC:
        case MOD_CLK_PERIPHERAL_TIMER4:
        case MOD_CLK_PERIPHERAL_SPI2:
        case MOD_CLK_PERIPHERAL_PORTA:
        case MOD_CLK_PERIPHERAL_PORTB:
        case MOD_CLK_PERIPHERAL_PORTC:
        case MOD_CLK_PERIPHERAL_PORTD:
        case MOD_CLK_PERIPHERAL_PORTE:
        case MOD_CLK_PERIPHERAL_ARINC429R:
        case MOD_CLK_PERIPHERAL_BKP:
        case MOD_CLK_PERIPHERAL_ARINC429T:
        case MOD_CLK_PERIPHERAL_PORTF:
        case MOD_CLK_PERIPHERAL_EXT_BUS_CNTRL:
        case MOD_CLK_PERIPHERAL_SPI3: {
            set_bit(
                get_rst_clk_regs()->per_clock,
                clk_peripheral,
                value ? 1 : 0
            );
            return 0;
        }
    }

    return -1;
}
