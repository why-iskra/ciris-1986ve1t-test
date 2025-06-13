//
// Created by whyiskra on 2025-06-07.
//

#include "drv/udpsrv/arptable.h"
#include "drv/clock.h"
#include "drv/udpsrv/utils.h"

struct arp_table_element {
    struct mod_eth_mac mac;
    struct drv_udpsrv_ip ip;
    uint32_t timestamp;
    bool valid;
};

static struct arp_table_element arp_table[ARP_TABLE_SIZE];

void arp_table_init(void) {
    for (size_t i = 0; i < ARP_TABLE_SIZE; i++) {
        arp_table[i].valid = false;
    }
}

bool arp_table_get_ip(struct drv_udpsrv_ip ip, struct mod_eth_mac *mac) {
    if (ip_eq(ip, BROADCAST_IP)) {
        *mac = BROADCAST_MAC;
        return true;
    }

    for (size_t i = 0; i < ARP_TABLE_SIZE; i++) {
        struct arp_table_element *elem = arp_table + i;
        if (elem->valid && ip_eq(ip, elem->ip)) {
            *mac = elem->mac;
            return true;
        }
    }

    return false;
}

void arp_table_set_ip(struct drv_udpsrv_ip ip, struct mod_eth_mac mac) {
    uint32_t timestamp = clock_millis();

    uint32_t max = 0;
    size_t index = ARP_TABLE_SIZE;
    for (size_t i = 0; i < ARP_TABLE_SIZE; i++) {
        struct arp_table_element *elem = arp_table + i;

        if (!elem->valid) {
            index = i;
            break;
        }

        uint32_t diff = timestamp - elem->timestamp;
        if (max <= diff) {
            max = diff;
            index = i;
        }
    }

    arp_table[index] = (struct arp_table_element) {
        .ip = ip,
        .mac = mac,
        .timestamp = clock_millis(),
        .valid = true,
    };
}

void arp_table_validate(void) {
    uint32_t timestamp = clock_millis();
    for (size_t i = 0; i < ARP_TABLE_SIZE; i++) {
        struct arp_table_element *elem = arp_table + i;

        if (!elem->valid) {
            continue;
        }

        uint32_t diff = timestamp - elem->timestamp;
        if (diff / 1000 > ARP_TABLE_LIVE_SEC) {
            elem->valid = false;
        }
    }
}
