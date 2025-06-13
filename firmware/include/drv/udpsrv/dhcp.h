//
// Created by whyiskra on 2025-06-08.
//

#pragma once

#include "drv/udpsrv.h"
#include <stdbool.h>

struct dhcp_offer {
    uint32_t rent_time;
    struct drv_udpsrv_ip server;
    struct drv_udpsrv_ip ip;
    struct drv_udpsrv_ip mask;
};

void dhcp_discover(struct mod_eth_frame *, uint32_t, struct drv_udpsrv_ip);
bool dhcp_offer(struct mod_eth_frame *, uint32_t, struct dhcp_offer *);
void dhcp_request(struct mod_eth_frame *, uint32_t, struct dhcp_offer);
bool dhcp_ack(struct mod_eth_frame *, uint32_t, struct dhcp_offer *);
