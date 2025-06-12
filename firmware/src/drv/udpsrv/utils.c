//
// Created by whyiskra on 2025-06-07.
//

#include "drv/udpsrv/utils.h"
#include <memory.h>

bool mac_eq(struct mod_eth_mac a, struct mod_eth_mac b) {
    return memcmp(&a, &b, sizeof(struct mod_eth_mac)) == 0;
}

bool ip_eq(struct drv_udpsrv_ip a, struct drv_udpsrv_ip b) {
    return memcmp(&a, &b, sizeof(struct drv_udpsrv_ip)) == 0;
}
