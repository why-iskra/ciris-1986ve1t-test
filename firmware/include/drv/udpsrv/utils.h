//
// Created by whyiskra on 2025-06-07.
//

#pragma once

#include "drv/udpsrv.h"
#include <stdbool.h>

bool mac_eq(struct mod_eth_mac, struct mod_eth_mac);
bool ip_eq(struct drv_udpsrv_ip, struct drv_udpsrv_ip);
struct drv_udpsrv_ip ip_broadcast_mask(struct drv_udpsrv_ip, struct drv_udpsrv_ip);
