//
// Created by whyiskra on 2025-06-07.
//

#pragma once

#include "drv/udpsrv.h"
#include <stdbool.h>

#define ARP_TABLE_SIZE     (8)
#define ARP_TABLE_LIVE_SEC (60)

void arp_init(void);

bool arp_table_get_ip(struct drv_udpsrv_ip, struct mod_eth_mac *);
void arp_table_set_ip(struct drv_udpsrv_ip, struct mod_eth_mac);
void arp_table_validate(void);

bool arp_external_request(struct mod_eth_frame *, struct drv_udpsrv_ip);

void arp_request(
    struct mod_eth_frame *frame,
    struct drv_udpsrv_ip src_ip,
    struct drv_udpsrv_ip dest_ip
);

bool arp_answer(
    struct mod_eth_frame *,
    struct drv_udpsrv_ip,
    struct mod_eth_mac *
);
