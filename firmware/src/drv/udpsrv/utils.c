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

struct drv_udpsrv_ip ip_mask(struct drv_udpsrv_ip ip, struct drv_udpsrv_ip mask) {
    return (struct drv_udpsrv_ip) {
        .octet1 = ip.octet1 & mask.octet1,
        .octet2 = ip.octet2 & mask.octet2,
        .octet3 = ip.octet3 & mask.octet3,
        .octet4 = ip.octet4 & mask.octet4,
    };
}
