//
// Created by whyiskra on 2025-05-24.
//

#include "drv/clock.h"
#include "drv/lcd.h"
#include "drv/terminal.h"
#include "mod/clk.h"
#include "mod/eth.h"
#include "mod/nvic.h"
#include "mod/port.h"
#include "mod/regs.h"
#include "mod/watchdog.h"
#include "utils/bits.h"
#include <stdio.h>
#include <stdlib.h>

static struct mod_eth_mac mac = {
    .value = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc }
};

static struct mod_eth_frame frame = { .received = 0 };

void __isr_ethernet(void);
void __isr_ethernet(void) {
    struct eth_regs *regs = get_eth_regs();
    uint16_t status = regs->ifr;

    if (get_bit(status, 0)) {
        port_write(MOD_PORT_D, 8, 1);
        read_packet(&frame);
        port_write(MOD_PORT_D, 8, 0);
    }

    regs->ifr = status;
}

int main(void) {
    clk_set_cpu_freq(MOD_CLK_CPU_FREQ_96MHZ);

    drv_clock_init();
    drv_lcd_init();
    drv_terminal_init();

    struct mod_port_cfg cfg = port_default_cfg();
    cfg.func = MOD_PORT_FUNC_PORT;
    cfg.mode = MOD_PORT_MODE_DIGITAL;
    cfg.dir = MOD_PORT_DIR_OUT;
    cfg.speed = MOD_PORT_SPEED_FAST;

    clk_en_peripheral(MOD_CLK_PERIPHERAL_PORTB, true);
    clk_en_peripheral(MOD_CLK_PERIPHERAL_PORTD, true);
    port_setup(MOD_PORT_B, 14, cfg);
    port_setup(MOD_PORT_B, 15, cfg);
    port_setup(MOD_PORT_D, 7, cfg);
    port_setup(MOD_PORT_D, 8, cfg);

    eth_setup(mac);
    nvic_irq_en(MOD_NVIC_IRQ_ETHERNET, true);

    while (true) {
        terminal_clear();
        size_t size = frame.received;
        for (size_t i = 0; i < size; i++) {
            printf("%02x ", frame.payload[i]);
        }
        fflush(stdout);

        iwdt_notify();
        clock_delay(250, DRV_CLOCK_UNIT_MS);
    }
}
