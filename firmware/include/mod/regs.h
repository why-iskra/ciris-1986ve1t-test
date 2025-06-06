//
// Created by whyiskra on 2025-05-24.
//

#pragma once

#include <stdint.h>

#define PORT_A_BASE (0x400A8000)
#define PORT_B_BASE (0x400B0000)
#define PORT_C_BASE (0x400B8000)
#define PORT_D_BASE (0x400C0000)
#define PORT_E_BASE (0x400C8000)
#define PORT_F_BASE (0x400E8000)

#define RST_CLK_BASE (0x40020000)

#define SCB_SYSTICK_BASE (0xE000E010)

#define TIMER_1_BASE (0x40070000)
#define TIMER_2_BASE (0x40078000)
#define TIMER_3_BASE (0x40080000)
#define TIMER_4_BASE (0x40098000)

#define NVIC_ISER_BASE (0xE000E100)
#define NVIC_ICER_BASE (0xE000E180)
#define NVIC_ISPR_BASE (0xE000E200)
#define NVIC_ICPR_BASE (0xE000E280)

#define EXTBUS_BASE (0x400F0000)

#define AIRCR_BASE (0xE000ED0C)

#define IWDT_BASE (0x40068000)

#define ETH_BASE     (0x30000000)
#define ETH_BUF_BASE (0x38000000)

// access

#define get_nvic_reg(x)        ((volatile uint32_t *) NVIC_##x##_BASE)
#define get_aircr_reg()        ((volatile uint32_t *) AIRCR_BASE)
#define get_rst_clk_regs()     ((struct rst_clk_regs *) RST_CLK_BASE)
#define get_port_regs(x)       ((struct port_regs *) PORT_##x##_BASE)
#define get_scb_systick_regs() ((struct scb_systick_regs *) SCB_SYSTICK_BASE)
#define get_timer_regs(x)      ((struct timer_regs *) TIMER_##x##_BASE)
#define get_extbus_regs()      ((struct extbus_regs *) EXTBUS_BASE)
#define get_iwdt_regs()        ((struct iwdt_regs *) IWDT_BASE)
#define get_eth_regs()         ((struct eth_regs *) ETH_BASE)
#define get_eth_buf(x)         ((volatile uint32_t *) (ETH_BUF_BASE + (x)))

// structures

struct rst_clk_regs {
    volatile uint32_t clock_status;  // 0x00
    volatile uint32_t pll_control;   // 0x04
    volatile uint32_t hs_control;    // 0x08
    volatile uint32_t cpu_clock;     // 0x0C
    volatile uint32_t usb_clock;     // 0x10
    volatile uint32_t adc_mco_clock; // 0x14
    volatile uint32_t rtc_clock;     // 0x18
    volatile uint32_t per_clock;     // 0x1C
    volatile uint32_t can_clock;     // 0x20
    volatile uint32_t tim_clock;     // 0x24
    volatile uint32_t uart_clock;    // 0x28
    volatile uint32_t ssp_clock;     // 0x2C
    volatile uint32_t __reserved;    // 0x30
    volatile uint32_t eth_clock;     // 0x34
};

struct port_regs {
    volatile uint32_t rxtx;   // 0x00
    volatile uint32_t oe;     // 0x04
    volatile uint32_t func;   // 0x08
    volatile uint32_t analog; // 0x0C
    volatile uint32_t pull;   // 0x10
    volatile uint32_t pd;     // 0x14
    volatile uint32_t pwr;    // 0x18
    volatile uint32_t gfen;   // 0x1C
    volatile uint32_t settx;  // 0x20
    volatile uint32_t clrtx;  // 0x24
    volatile uint32_t rdtx;   // 0x28
};

struct scb_systick_regs {
    volatile uint32_t systick_ctrl;  // 0x10
    volatile uint32_t systick_load;  // 0x14
    volatile uint32_t systick_val;   // 0x18
    volatile uint32_t systick_calib; // 0x1C
};

struct timer_regs {
    volatile uint32_t cnt;          // 0x00
    volatile uint32_t psg;          // 0x04
    volatile uint32_t arr;          // 0x08
    volatile uint32_t cntrl;        // 0x0c
    volatile uint32_t ccr[4];       // 0x10
    volatile uint32_t ch_cntrl[4];  // 0x20
    volatile uint32_t ch_cntrl1[4]; // 0x30
    volatile uint32_t ch_dtg[4];    // 0x40
    volatile uint32_t brketr_cntrl; // 0x50
    volatile uint32_t status;       // 0x54
    volatile uint32_t ie;           // 0x58
    volatile uint32_t dma_re;       // 0x5c
    volatile uint32_t ch_cntrl2[4]; // 0x60
    volatile uint32_t ccr1[4];      // 0x70
    volatile uint32_t dma_rex[4];   // 0x80
};

struct extbus_regs {
    volatile uint32_t __reserved[20]; // 0x00
    volatile uint32_t nand_cycles;    // 0x50
    volatile uint32_t control;        // 0x54
    volatile uint32_t ram_cycles1;    // 0x58
    volatile uint32_t ram_cycles2;    // 0x5c
    volatile uint32_t ram_cycles3;    // 0x60
    volatile uint32_t ram_cycles4;    // 0x64
};

struct iwdt_regs {
    volatile uint32_t kr;  // 0x00
    volatile uint32_t pr;  // 0x04
    volatile uint32_t prl; // 0x08
    volatile uint32_t sr;  // 0x0c
};

struct eth_regs {
    volatile uint16_t delimiter;  // 0x00
    volatile uint16_t mac[3];     // 0x02
    volatile uint16_t hash[4];    // 0x08
    volatile uint16_t ipg;        // 0x10
    volatile uint16_t psc;        // 0x12
    volatile uint16_t bag;        // 0x14
    volatile uint16_t jw;         // 0x16
    volatile uint16_t r_cfg;      // 0x18
    volatile uint16_t x_cfg;      // 0x1a
    volatile uint32_t g_cfg;      // 0x1c
    volatile uint16_t imr;        // 0x20
    volatile uint16_t ifr;        // 0x22
    volatile uint16_t mdio_ctrl;  // 0x24
    volatile uint16_t mdio_data;  // 0x26
    volatile uint16_t r_head;     // 0x28
    volatile uint16_t x_tail;     // 0x2a
    volatile uint16_t r_tail;     // 0x2c
    volatile uint16_t x_head;     // 0x2e
    volatile uint16_t stat;       // 0x30
    volatile uint16_t __reserved; // 0x32
    volatile uint16_t phy_ctrl;   // 0x34
    volatile uint16_t phy_status; // 0x36
};
