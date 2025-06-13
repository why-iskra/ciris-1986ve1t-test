//
// Created by whyiskra on 2025-06-07.
//

#include "drv/udpsrv/arp.h"
#include "drv/clock.h"
#include "drv/udpsrv/frames.h"
#include "drv/udpsrv/utils.h"

struct arp_table_element {
    struct mod_eth_mac mac;
    struct drv_udpsrv_ip ip;
    uint32_t timestamp;
    bool valid;
};

static struct arp_table_element arp_table[ARP_TABLE_SIZE];

void arp_init(void) {
    for (size_t i = 0; i < ARP_TABLE_SIZE; i++) {
        arp_table[i].valid = false;
    }
}

bool arp_table_get_ip(
    struct drv_udpsrv_ip search_ip,
    struct drv_udpsrv_ip ip,
    struct drv_udpsrv_ip mask,
    struct mod_eth_mac *mac
) {
    if (ip_eq(search_ip, BROADCAST_IP)) {
        *mac = BROADCAST_MAC;
        return true;
    }

    if (ip_eq(search_ip, ip_broadcast_mask(ip, mask))) {
        *mac = BROADCAST_MAC;
        return true;
    }

    for (size_t i = 0; i < ARP_TABLE_SIZE; i++) {
        struct arp_table_element *elem = arp_table + i;
        if (elem->valid && ip_eq(search_ip, elem->ip)) {
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

static bool eth_arp_validate(struct mod_eth_frame *frame, arp_opcode_t opcode) {
    if (frame->size < sizeof(struct arp_frame)) {
        return false;
    }

    struct ethernet_frame *ethernet_frame =
        (struct ethernet_frame *) frame->payload;

    if (!mac_eq(ethernet_frame->dest_mac, eth_mac())
        && !mac_eq(ethernet_frame->dest_mac, BROADCAST_MAC)) {
        return false;
    }

    if (ethernet_frame->ethertype != ETHERTYPE_ARP) {
        return false;
    }

    if (!frame_valid_arp(frame, opcode)) {
        return false;
    }

    return true;
}

bool arp_external_request(struct mod_eth_frame *frame, struct drv_udpsrv_ip ip) {
    if (!eth_arp_validate(frame, ARP_OPCODE_REQUEST)) {
        return false;
    }

    struct arp_frame *arp_frame = (struct arp_frame *) frame->payload;

    if (!ip_eq(ip, arp_frame->target_ip)) {
        return false;
    }

    struct drv_udpsrv_ip sender_ip = arp_frame->sender_ip;
    struct mod_eth_mac sender_mac = arp_frame->sender_mac;

    frame_setup_ethernet(frame, sender_mac, ETHERTYPE_ARP);

    frame_setup_arp(
        frame,
        ARP_OPCODE_ANSWER,
        eth_mac(),
        ip,
        sender_mac,
        sender_ip
    );

    frame->size = sizeof(struct arp_frame);

    return true;
}

void arp_request(
    struct mod_eth_frame *frame,
    struct drv_udpsrv_ip src_ip,
    struct drv_udpsrv_ip dest_ip
) {
    frame_setup_ethernet(frame, BROADCAST_MAC, ETHERTYPE_ARP);

    frame_setup_arp(
        frame,
        ARP_OPCODE_REQUEST,
        eth_mac(),
        src_ip,
        ZERO_MAC,
        dest_ip
    );

    frame->size = sizeof(struct arp_frame);
}

bool arp_answer(
    struct mod_eth_frame *frame,
    struct drv_udpsrv_ip ip,
    struct mod_eth_mac *mac
) {
    if (!eth_arp_validate(frame, ARP_OPCODE_ANSWER)) {
        return false;
    }

    struct arp_frame *arp_frame = (struct arp_frame *) frame->payload;

    if (!mac_eq(eth_mac(), arp_frame->target_mac)) {
        return false;
    }

    if (!ip_eq(ip, arp_frame->target_ip)) {
        return false;
    }

    arp_table_set_ip(arp_frame->sender_ip, arp_frame->sender_mac);

    if (mac != NULL) {
        *mac = arp_frame->sender_mac;
    }

    return true;
}
