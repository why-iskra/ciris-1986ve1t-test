//
// Created by whyiskra on 2025-05-30.
//

#include "mod/eth.h"
#include "mod/regs.h"
#include "utils/bits.h"
#include <stddef.h>

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

void eth_setup(struct mod_eth_mac mac) {
    struct eth_regs *eth_regs = get_eth_regs();
    struct rst_clk_regs *clk_regs = get_rst_clk_regs();

    set_bit(clk_regs->hs_control, 2, 1);
    while (get_bit(clk_regs->clock_status, 3)) { }
    set_bit(clk_regs->eth_clock, 24, 1);
    set_bit(clk_regs->eth_clock, 27, 1);
    set_bits(clk_regs->eth_clock, 28, 0x3, 3);

    phy_setup(0x1c, 0x3);

    for (size_t i = 0; i < sizeof(mac.value); i++) {
        ((volatile uint8_t *) eth_regs->mac)[i] = mac.value[i];
    }

    mac_setup();
}

// uint32_t read_packet(struct mod_eth_frame *frame) {
//     struct eth_regs *regs = get_eth_regs();
//
//     uint16_t tail = regs->r_tail;
//     uint16_t head = regs->r_head;
//     uint16_t space_start = 0;
//     uint16_t space_end = 0;
//
//     if (tail > head) {
//         space_end = tail - head;
//         space_start = 0;
//     } else {
//         space_end = 0x1000 - head;
//         space_start = tail;
//     }
//
//     uint32_t *src = (uint32_t *) (ETH_BUF_BASE + head);
//     uint32_t *dest = (uint32_t *) (frame->data);
//
//     uint16_t tmp[2];
//
//     *((uint32_t *) tmp) = *src++;
//     space_end -= 4;
//
//     if ((((uintptr_t) src) & 0x0000FFFF) > 0xFFF) {
//         src = (uint32_t *) ETH_BUF_BASE;
//     }
//
//     size_t size = (tmp[0] + 3) / 4;
//     if (tmp[0] <= space_end) {
//         for (size_t i = 0; i < size; i++) {
//             *dest++ = *src++;
//         }
//     } else {
//         size = size - space_end / 4;
//         for (size_t i = 0; i < (space_end / 4); i++) {
//             *dest++ = *src++;
//         }
//
//         src = (uint32_t *) ETH_BUF_BASE;
//         for (size_t i = 0; i < size; i++) {
//             *dest++ = *src++;
//         }
//     }
//
//     if ((((uintptr_t) src) & 0x0000FFFF) > 0xFFF) {
//         src = (uint32_t *) ETH_BUF_BASE;
//     }
//
//     regs->r_head = (uint16_t) (((uintptr_t) src) & 0x0000FFFF);
//     regs->stat -= 0x20;
//
//     return tmp[0];
// }

int send_packet(const void *data, size_t size) {
    struct eth_regs *regs = get_eth_regs();

    if (size > 3072) {
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
