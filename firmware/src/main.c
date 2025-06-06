//
// Created by whyiskra on 2025-05-24.
//

#include "drv/clock.h"
#include "drv/lcd.h"
#include "drv/terminal.h"
#include "mod/clk.h"
#include "mod/nvic.h"
#include "mod/eth.h"
#include "mod/port.h"
#include "mod/regs.h"
#include "mod/watchdog.h"
#include "utils/bits.h"
#include <stdio.h>
#include <stdlib.h>

static struct mod_eth_mac mac = {
    .value = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc }
};

struct frame {
    uint32_t data[768];
    size_t received;
};

void __isr_ethernet(void);
void __isr_ethernet(void) {
    uint16_t status = get_eth_regs()->ifr;
    if (get_bit(status, 0)) {
        port_write(MOD_PORT_B, 14, !port_read(MOD_PORT_B, 14));
    }
}

// static void read_packet(struct frame *frame) {
//     struct eth_regs *regs = get_eth_regs();
//
//     uint16_t src = regs->r_head;
//     uint16_t tail = regs->r_tail;
//
//     size_t received = 0;
//     for (; src != tail && received < sizeof(frame->data); received ++) {
//         frame->data[received] = *get_eth_buf(src);
//         src = (uint16_t) (src + 4) % 0x1000;
//     }
//
//     regs->r_head = tail;
//     regs->stat -= 0x20;
//
//     frame->received = received;
// }

struct ethernet_frame {
    uint8_t dest_mac[6];
    uint8_t src_mac[6];
    uint16_t ethertype;
};

struct arp_frame {
    struct ethernet_frame ethernet;
    uint16_t hardware_type;
    uint16_t protocol_type;
    uint8_t hardware_size;
    uint8_t protocol_size;
    uint16_t opcode;
    uint8_t sender_mac[6];
    uint8_t sender_ip[4];
    uint8_t target_mac[6];
    uint8_t target_ip[4];
};

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

    eth_setup(mac);
    // nvic_irq_en(MOD_NVIC_IRQ_ETHERNET, true);

    struct arp_frame data = {
        .ethernet.dest_mac = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
        .ethernet.src_mac = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc },
        .ethernet.ethertype = 0x0608,
        .hardware_type = 0x0100,
        .protocol_type = 0x0008,
        .hardware_size = 0x06,
        .protocol_size = 0x04,
        .opcode = 0x0100,
        .sender_mac = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc },
        .sender_ip = { 0xc0, 0xa8, 0x01, 0x12 },
        .target_mac = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        .target_ip = { 0xc0, 0xa8, 0x01, 0x01 },
    };

    port_write(MOD_PORT_D, 7, 1);

    int i = 0;
    while (true) {
        terminal_clear();
        printf("%d", i);
        fflush(stdout);

        send_packet(&data, sizeof(struct arp_frame));
        iwdt_notify();
        i ++;
    }
}
