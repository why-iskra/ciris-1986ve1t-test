//
// Created by whyiskra on 2025-05-24.
//

#include "drv/clock.h"
#include "drv/lcd.h"
#include "drv/terminal.h"
#include "drv/udpsrv.h"
#include "mod/clk.h"
#include "mod/watchdog.h"

static struct mod_eth_mac mac = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc };

int main(void) {
    clk_set_cpu_freq(MOD_CLK_CPU_FREQ_96MHZ);

    drv_clock_init();
    drv_lcd_init();
    drv_terminal_init();
    drv_udpsrv_init(mac, NULL);

    while (true) {
        iwdt_notify();
        clock_delay(250, DRV_CLOCK_UNIT_MS);
    }
}
