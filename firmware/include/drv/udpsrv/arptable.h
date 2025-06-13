//
// Created by whyiskra on 2025-06-07.
//

#pragma once

#include <stdbool.h>
#include "drv/udpsrv.h"

#define ARP_TABLE_SIZE     (8)
#define ARP_TABLE_LIVE_SEC (60)

void arp_table_init(void);
bool arp_table_get_ip(struct drv_udpsrv_ip, struct mod_eth_mac *);
void arp_table_set_ip(struct drv_udpsrv_ip, struct mod_eth_mac);
void arp_table_validate(void);
