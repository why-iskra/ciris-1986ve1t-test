//
// Created by whyiskra on 2025-05-30.
//

#include "mod/eth.h"
#include "mod/regs.h"
#include "utils/bits.h"
#include <stdbool.h>
#include <stddef.h>

static mod_eth_handler_t handler = NULL;

static void phy_setup(uint8_t address, uint8_t mode) {
    struct eth_regs *regs = get_eth_regs();
    uint16_t ctrl = regs->phy_ctrl;
    ctrl &= 0x0770;
    set_bit(ctrl, 0, 1);
    set_bits(ctrl, 1, 0x7, mode);
    set_bits(ctrl, 11, 0x1f, address);

    regs->phy_ctrl = ctrl;

    while (!get_bit(regs->phy_status, 4)) { }
}

static void clear_buffer(void) {
    for (size_t i = 0; i < 2048; i++) {
        get_eth_buf(0)[i] = 0;
    }
}

static void mac_setup(void) {
    struct eth_regs *regs = get_eth_regs();
    set_bit(regs->g_cfg, 16, 1);
    set_bit(regs->g_cfg, 17, 1);

    clear_buffer();

    regs->delimiter = 0x1000;

    regs->hash[0] = 0;
    regs->hash[1] = 0;
    regs->hash[2] = 0;
    regs->hash[3] = 0x8000;

    regs->ipg = 0x60;
    regs->psc = 0x50;
    regs->bag = 0x200;
    regs->jw = 0x5;

    set_bit(regs->r_cfg, 1, 1);
    set_bit(regs->r_cfg, 2, 1);
    set_bits(regs->r_cfg, 8, 0x7, 0x4);
    set_bit(regs->r_cfg, 15, 1);

    set_bits(regs->x_cfg, 0, 0xf, 10);
    set_bit(regs->x_cfg, 4, 1);
    set_bit(regs->x_cfg, 5, 1);
    set_bit(regs->x_cfg, 6, 1);
    set_bit(regs->x_cfg, 7, 1);
    set_bits(regs->x_cfg, 8, 0x7, 0x1);
    set_bit(regs->x_cfg, 15, 1);

    regs->g_cfg = 0;
    set_bits(regs->g_cfg, 0, 0xff, 128);
    set_bits(regs->g_cfg, 12, 0x3, 0);
    set_bit(regs->g_cfg, 28, 1);
    set_bit(regs->g_cfg, 29, 1);
    set_bit(regs->g_cfg, 16, 1);
    set_bit(regs->g_cfg, 17, 1);

    regs->imr = 0;
    regs->ifr = 0xFFFF;

    regs->r_head = 0x0000;
    regs->x_tail = 0x1000;

    set_bit(regs->g_cfg, 14, 0);
    set_bit(regs->g_cfg, 16, 0);
    set_bit(regs->g_cfg, 17, 0);

    regs->imr = 0x0101;
}

void __isr_ethernet(void);
void __isr_ethernet(void) {
    struct eth_regs *regs = get_eth_regs();
    uint16_t status = regs->ifr;
    if (get_bit(status, 0) && handler != NULL) {
        handler();
    }
    regs->ifr = status;
}

void eth_setup(struct mod_eth_mac mac) {
    handler = NULL;

    struct eth_regs *eth_regs = get_eth_regs();
    struct rst_clk_regs *clk_regs = get_rst_clk_regs();

    set_bit(clk_regs->hs_control, 2, 1);
    while (get_bit(clk_regs->clock_status, 3)) { }
    set_bit(clk_regs->eth_clock, 24, 1);
    set_bit(clk_regs->eth_clock, 27, 1);
    set_bits(clk_regs->eth_clock, 28, 0x3, 3);

    phy_setup(0x1c, 0x3);

    for (size_t i = 0; i < sizeof(mac); i++) {
        ((volatile uint8_t *) eth_regs->mac)[i] = ((uint8_t *) &mac)[i];
    }

    mac_setup();
}

void eth_set_handler(mod_eth_handler_t func) {
    handler = func;
}

struct mod_eth_mac eth_mac(void) {
    return *((volatile struct mod_eth_mac *) get_eth_regs()->mac);
}

int eth_receive(struct mod_eth_frame *frame) {
    struct eth_regs *regs = get_eth_regs();

    uint16_t src = regs->r_head;
    uint16_t tail = regs->r_tail;

    bool error = get_bits(regs->stat, 5, 0x3) == 0;

    if (!error) {
        uint32_t status = *get_eth_buf(src);
        src = (uint16_t) (src + 4) % 0x1000;

        size_t size = status & 0x0000FFFF;
        size_t received = 0;
        for (; received < size && received < MOD_ETH_PAYLOAD_SIZE; received++) {
            frame->payload_word[received] = *get_eth_buf(src);
            src = (uint16_t) (src + 4) % 0x1000;
        }

        regs->r_head = tail;
        regs->stat -= 0x20;

        error = size > MOD_ETH_PAYLOAD_SIZE;
        frame->received = received;
    } else {
        frame->received = 0;
    }

    return error ? -1 : 0;
}

int eth_send(const void *data, size_t size) {
    struct eth_regs *regs = get_eth_regs();

    if (size > MOD_ETH_PAYLOAD_SIZE) {
        return -1;
    }

    uint16_t dest = regs->x_tail;

    *get_eth_buf(dest) = size;
    dest = (uint16_t) ((dest + 4) - 0x1000) % 0x1000 + 0x1000;

    for (size_t i = 0; i < size;) {
        uint32_t word = 0;

        for (size_t k = 0; k < sizeof(word) && i < size; k++, i++) {
            word |= ((uint32_t) ((const uint8_t *) data)[i]) << (k * 8);
        }

        *get_eth_buf(dest) = word;
        dest = (uint16_t) ((dest + 4) - 0x1000) % 0x1000 + 0x1000;
    }

    *get_eth_buf(dest) = 0;
    dest = (uint16_t) ((dest + 4) - 0x1000) % 0x1000 + 0x1000;

    regs->x_tail = dest;
    return 0;
}
