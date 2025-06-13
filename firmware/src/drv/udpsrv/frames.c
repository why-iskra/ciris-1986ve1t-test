//
// Created by whyiskra on 2025-06-07.
//

#include "drv/udpsrv/frames.h"
#include "drv/clock.h"
#include <memory.h>

struct udp_ipv4_pseudo_header {
    struct drv_udpsrv_ip src;
    struct drv_udpsrv_ip dest;
    uint8_t zeroes;
    uint8_t protocol;
    uint16_t udp_length;
} __attribute__((packed));

static uint32_t checksum_sum(uint8_t *bytes, size_t length) {
    uint32_t sum = 0;

    for (size_t i = 0; i < length; i += 2) {
        sum += (uint16_t) ((uint16_t) bytes[i] << 8)
               | (((i + 1) < length) ? bytes[i + 1] : 0);
    }

    return sum;
}

static uint16_t checksum(
    uint8_t *header,
    size_t header_length,
    uint8_t *bytes,
    size_t length
) {
    uint32_t sum =
        checksum_sum(header, header_length) + checksum_sum(bytes, length);

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~((uint16_t) sum);
}

static void reverse_bytes(void *data, size_t length) {
    uint8_t *array = data;
    for (size_t i = 0; i < length / 2; i++) {
        uint8_t temp = array[i];
        array[i] = array[length - 1 - i];
        array[length - 1 - i] = temp;
    }
}

void frame_setup_ethernet(
    struct mod_eth_frame *frame,
    struct mod_eth_mac dest_mac,
    ethertype_t ethertype
) {
    struct ethernet_frame *result = (struct ethernet_frame *) frame->payload;

    result->dest_mac = dest_mac;
    result->src_mac = eth_mac();
    result->ethertype = (uint16_t) ethertype;
}

void frame_setup_arp(
    struct mod_eth_frame *frame,
    arp_opcode_t opcode,
    struct mod_eth_mac sender_mac,
    struct drv_udpsrv_ip sender_ip,
    struct mod_eth_mac target_mac,
    struct drv_udpsrv_ip target_ip
) {
    struct arp_frame *result = (struct arp_frame *) frame->payload;

    result->hardware_type = ARP_HARDWARE_TYPE_ETHERNET;
    result->protocol_type = ARP_PROTOCOL_TYPE_IPV4;
    result->hardware_size = sizeof(struct mod_eth_mac);
    result->protocol_size = sizeof(struct drv_udpsrv_ip);
    result->opcode = (uint16_t) opcode;
    result->sender_mac = sender_mac;
    result->sender_ip = sender_ip;
    result->target_mac = target_mac;
    result->target_ip = target_ip;
}

bool frame_valid_arp(struct mod_eth_frame *frame, arp_opcode_t opcode) {
    struct arp_frame *result = (struct arp_frame *) frame->payload;

    if (result->hardware_type != ARP_HARDWARE_TYPE_ETHERNET
        || result->hardware_size != sizeof(struct mod_eth_mac)) {
        return false;
    }

    if (result->protocol_type != ARP_PROTOCOL_TYPE_IPV4
        || result->protocol_size != sizeof(struct drv_udpsrv_ip)) {
        return false;
    }

    return result->opcode == opcode;
}

void frame_setup_ipv4(
    struct mod_eth_frame *frame,
    ipv4_protocol_t protocol,
    struct drv_udpsrv_ip src,
    struct drv_udpsrv_ip dest
) {
    struct ipv4_frame *result = (struct ipv4_frame *) frame->payload;

    result->version = 0x45;
    result->dos = 0;
    result->length = 0;
    result->id = (uint16_t) clock_millis();
    result->fragment = 0x0040; // don't fragment
    result->ttl = IPV4_TTL;
    result->protocol = (uint8_t) protocol;
    result->checksum = 0;
    result->src_ip = src;
    result->dest_ip = dest;
}

bool frame_valid_ipv4(struct mod_eth_frame *frame, ipv4_protocol_t protocol) {
    struct ipv4_frame *result = (struct ipv4_frame *) frame->payload;

    uint16_t length = result->length;
    reverse_bytes(&length, sizeof(length));

    if (result->version != 0x45 || length < IPV4_FRAME_SIZE) {
        return false;
    }

    if ((result->fragment & 0x00A0) != 0) {
        return false;
    }

    if (result->protocol != protocol) {
        return false;
    }

    uint16_t got_checksum = result->checksum;

    result->checksum = 0;

    uint16_t expected_checksum = checksum(
        NULL,
        0,
        ((uint8_t *) result) + ETHERNET_FRAME_SIZE,
        IPV4_FRAME_SIZE
    );

    result->checksum = got_checksum;

    reverse_bytes(&expected_checksum, sizeof(expected_checksum));

    return expected_checksum == got_checksum;
}

void frame_setup_udp(
    struct mod_eth_frame *frame,
    uint16_t src_port,
    uint16_t dest_port,
    size_t length
) {
    struct udp_frame *result = (struct udp_frame *) frame->payload;

    result->src_port = src_port;
    result->dest_port = dest_port;
    result->length = ((uint16_t) length) + UDP_FRAME_SIZE;
    result->checksum = 0;

    result->ipv4.length =
        ((uint16_t) length) + IPV4_FRAME_SIZE + UDP_FRAME_SIZE;

    reverse_bytes(&result->length, sizeof(result->length));
    reverse_bytes(&result->src_port, sizeof(result->src_port));
    reverse_bytes(&result->dest_port, sizeof(result->dest_port));
    reverse_bytes(&result->ipv4.length, sizeof(result->ipv4.length));

    struct udp_ipv4_pseudo_header ph = {
        .src = result->ipv4.src_ip,
        .dest = result->ipv4.dest_ip,
        .zeroes = 0,
        .protocol = result->ipv4.protocol,
        .udp_length = result->length,
    };

    result->checksum = checksum(
        (uint8_t *) &ph,
        sizeof(ph),
        ((uint8_t *) result) + ETHERNET_FRAME_SIZE + IPV4_FRAME_SIZE,
        length + UDP_FRAME_SIZE
    );

    reverse_bytes(&result->checksum, sizeof(result->checksum));

    result->ipv4.checksum = checksum(
        NULL,
        0,
        ((uint8_t *) result) + ETHERNET_FRAME_SIZE,
        IPV4_FRAME_SIZE
    );

    reverse_bytes(&result->ipv4.checksum, sizeof(result->ipv4.checksum));
}

bool frame_valid_udp(
    struct mod_eth_frame *frame,
    bool check_src_port,
    uint16_t expected_src_port,
    bool check_dest_port,
    uint16_t expected_dest_port
) {
    struct udp_frame *result = (struct udp_frame *) frame->payload;

    uint16_t got_checksum = result->checksum;
    uint16_t length = result->length;
    uint16_t src_port = result->src_port;
    uint16_t dest_port = result->dest_port;

    reverse_bytes(&length, sizeof(length));
    reverse_bytes(&src_port, sizeof(src_port));
    reverse_bytes(&dest_port, sizeof(dest_port));

    size_t size = ((size_t) length) + ETHERNET_FRAME_SIZE + IPV4_FRAME_SIZE;
    if (frame->size < size) {
        return false;
    }

    if (check_src_port && src_port != expected_src_port) {
        return false;
    }

    if (check_dest_port && dest_port != expected_dest_port) {
        return false;
    }

    if (got_checksum == 0) {
        return true;
    }

    struct udp_ipv4_pseudo_header ph = {
        .src = result->ipv4.src_ip,
        .dest = result->ipv4.dest_ip,
        .zeroes = 0,
        .protocol = result->ipv4.protocol,
        .udp_length = result->length,
    };

    result->checksum = 0;

    uint16_t expected_checksum = checksum(
        (uint8_t *) &ph,
        sizeof(ph),
        ((uint8_t *) result) + ETHERNET_FRAME_SIZE + IPV4_FRAME_SIZE,
        length
    );

    result->checksum = got_checksum;

    reverse_bytes(&expected_checksum, sizeof(expected_checksum));

    return expected_checksum == got_checksum;
}

void frame_setup_bootp(
    struct mod_eth_frame *frame,
    bootp_op_t op,
    uint32_t xid,
    struct drv_udpsrv_ip ciaddr,
    struct drv_udpsrv_ip yiaddr,
    struct drv_udpsrv_ip siaddr,
    struct drv_udpsrv_ip giaddr,
    struct mod_eth_mac chaddr
) {
    struct bootp_frame *result = (struct bootp_frame *) frame->payload;

    result->op = (uint8_t) op;
    result->htype = 0x01;
    result->hlen = 0x06;
    result->hops = 0;
    result->xid = xid;
    result->secs = 0;
    result->flags = 0;
    result->ciaddr = ciaddr;
    result->yiaddr = yiaddr;
    result->siaddr = siaddr;
    result->giaddr = giaddr;
    result->chaddr.mac = chaddr;
    memset(result->chaddr._reserved, 0, sizeof(result->chaddr._reserved));
    memset(result->sname, 0, sizeof(result->sname));
    memset(result->file, 0, sizeof(result->file));
}

bool frame_valid_bootp(struct mod_eth_frame *frame) {
    struct bootp_frame *result = (struct bootp_frame *) frame->payload;
    return result->htype == 0x01 && result->hlen == 0x06;
}
