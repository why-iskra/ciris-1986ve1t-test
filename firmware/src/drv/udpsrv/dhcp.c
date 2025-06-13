//
// Created by whyiskra on 2025-06-08.
//

#include "drv/udpsrv/dhcp.h"
#include "drv/udpsrv/frames.h"
#include "drv/udpsrv/utils.h"
#include <memory.h>

#define DHCP_MAGIC (0x63538263)

typedef enum {
    DHCP_PORT_SERVER = 67,
    DHCP_PORT_CLIENT = 68,
} dhcp_port_t;

typedef enum {
    DHCP_OPTION_MESSAGE = 53,
    DHCP_OPTION_REQUEST_IP = 50,
    DHCP_OPTION_RENT_TIME = 51,
    DHCP_OPTION_SERVER = 54,
    DHCP_OPTION_MASK = 1,
} dhcp_option_t;

typedef enum {
    DHCP_MESSAGE_DISCOVER = 1,
    DHCP_MESSAGE_OFFER = 2,
    DHCP_MESSAGE_REQUEST = 3,
    DHCP_MESSAGE_ACK = 5,
} dhcp_message_t;

struct dhcp_frame {
    struct bootp_frame bootp;
    uint32_t magic;
    uint8_t end;
} __attribute__((packed));

static uint8_t dhcp_option_length(dhcp_option_t option) {
    uint8_t length = 0;
    switch (option) {
        case DHCP_OPTION_MESSAGE: length = 1; break;
        case DHCP_OPTION_REQUEST_IP: length = 4; break;
        case DHCP_OPTION_RENT_TIME: length = 4; break;
        case DHCP_OPTION_SERVER: length = 4; break;
        case DHCP_OPTION_MASK: length = 4; break;
    }

    return length;
}

static bool dhcp_add_option(
    struct mod_eth_frame *frame,
    dhcp_option_t option,
    const void *data
) {
    uint8_t length = dhcp_option_length(option);
    if (length == 0) {
        return false;
    }

    uint8_t *options = frame->payload + sizeof(struct dhcp_frame) - 1;
    size_t options_size =
        sizeof(frame->payload) - (sizeof(struct dhcp_frame) - 1);

    size_t index = 0;
    while (index < options_size) {
        if (options[index] == 0xff) {
            break;
        }

        if (options[index] == 0) {
            index++;
            continue;
        }

        index++;
        if (index >= options_size) {
            break;
        }

        index += options[index];
        index++;
    }

    if (index + length + 3 >= options_size) {
        return false;
    }

    options[index] = (uint8_t) option;
    options[index + 1] = length;
    memcpy(options + index + 2, data, length);
    options[index + length + 2] = 0xff;

    return true;
}

static void *dhcp_get_option(struct mod_eth_frame *frame, dhcp_option_t option) {
    uint8_t length = dhcp_option_length(option);
    if (length == 0) {
        return NULL;
    }

    uint8_t *options = frame->payload + sizeof(struct dhcp_frame) - 1;
    size_t options_size =
        sizeof(frame->payload) - (sizeof(struct dhcp_frame) - 1);

    size_t index = 0;
    while (index < options_size) {
        if (options[index] == 0xff) {
            return NULL;
        }

        if (options[index] == 0) {
            index++;
            continue;
        }

        uint8_t raw_option = options[index];

        index++;
        if (index >= options_size) {
            return NULL;
        }

        uint8_t raw_length = options[index];
        index++;

        if (raw_option == option && raw_length == length) {
            break;
        }

        index += raw_length;
    }

    if (index + length > options_size) {
        return NULL;
    }

    return options + index;
}

static size_t dhcp_options_size(struct mod_eth_frame *frame) {
    uint8_t *options = frame->payload + sizeof(struct dhcp_frame) - 1;
    size_t options_size =
        sizeof(frame->payload) - (sizeof(struct dhcp_frame) - 1);

    size_t index = 0;
    while (index < options_size) {
        if (options[index] == 0xff) {
            return index + 1;
        }

        if (options[index] == 0) {
            index++;
            continue;
        }

        index++;
        if (index >= options_size) {
            break;
        }

        index += options[index];
        index++;
    }

    return options_size;
}

static void dhcp_add_option_message(
    struct mod_eth_frame *frame,
    dhcp_message_t message
) {
    uint8_t value = (uint8_t) message;
    dhcp_add_option(frame, DHCP_OPTION_MESSAGE, &value);
}

static void dhcp_add_option_request_ip(
    struct mod_eth_frame *frame,
    struct drv_udpsrv_ip ip
) {
    dhcp_add_option(frame, DHCP_OPTION_REQUEST_IP, &ip);
}

static void dhcp_add_option_server(
    struct mod_eth_frame *frame,
    struct drv_udpsrv_ip ip
) {
    dhcp_add_option(frame, DHCP_OPTION_SERVER, &ip);
}

static void dhcp_setup(struct mod_eth_frame *frame) {
    struct dhcp_frame *result = (struct dhcp_frame *) frame->payload;
    result->magic = DHCP_MAGIC;
    result->end = 0xff;
}

void dhcp_discover(
    struct mod_eth_frame *frame,
    uint32_t xid,
    struct drv_udpsrv_ip request_ip
) {
    frame_setup_ethernet(frame, BROADCAST_MAC, ETHERTYPE_IPV4);

    frame_setup_ipv4(frame, IPV4_PROTOCOL_UDP, ZERO_IP, BROADCAST_IP);

    frame_setup_bootp(
        frame,
        BOOTP_OP_REQUEST,
        xid,
        ZERO_IP,
        ZERO_IP,
        ZERO_IP,
        ZERO_IP,
        eth_mac()
    );

    dhcp_setup(frame);
    dhcp_add_option_message(frame, DHCP_MESSAGE_DISCOVER);
    dhcp_add_option_request_ip(frame, request_ip);

    size_t options_size = dhcp_options_size(frame);
    size_t dhcp_size = sizeof(struct dhcp_frame) - sizeof(struct udp_frame) - 1;
    size_t size = dhcp_size + options_size;
    frame_setup_udp(frame, DHCP_PORT_CLIENT, DHCP_PORT_SERVER, size);

    frame->size = sizeof(struct dhcp_frame) - 1 + options_size;
}

bool dhcp_offer(
    struct mod_eth_frame *frame,
    uint32_t xid,
    struct dhcp_offer *result
) {
    if (frame->size < sizeof(struct dhcp_frame)) {
        return false;
    }

    struct ethernet_frame *ethernet_frame =
        (struct ethernet_frame *) frame->payload;
    struct bootp_frame *bootp_frame = (struct bootp_frame *) frame->payload;
    struct dhcp_frame *dhcp_frame = (struct dhcp_frame *) frame->payload;

    if (!mac_eq(ethernet_frame->dest_mac, eth_mac())
        || ethernet_frame->ethertype != ETHERTYPE_IPV4) {
        return false;
    }

    if (!frame_valid_ipv4(frame, IPV4_PROTOCOL_UDP)) {
        return false;
    }

    if (!frame_valid_udp(frame, true, DHCP_PORT_SERVER, true, DHCP_PORT_CLIENT)) {
        return false;
    }

    if (!frame_valid_bootp(frame) || !mac_eq(bootp_frame->chaddr.mac, eth_mac())
        || bootp_frame->op != BOOTP_OP_REPLY || bootp_frame->xid != xid) {
        return false;
    }

    if (dhcp_frame->magic != DHCP_MAGIC) {
        return false;
    }

    uint8_t *message_p = dhcp_get_option(frame, DHCP_OPTION_MESSAGE);
    uint32_t *rent_time_p = dhcp_get_option(frame, DHCP_OPTION_RENT_TIME);
    struct drv_udpsrv_ip *server_p = dhcp_get_option(frame, DHCP_OPTION_SERVER);
    struct drv_udpsrv_ip *mask_p = dhcp_get_option(frame, DHCP_OPTION_MASK);

    if (message_p == NULL || rent_time_p == NULL || server_p == NULL
        || mask_p == NULL) {
        return false;
    }

    uint8_t message;
    memcpy(&message, message_p, sizeof(*message_p));

    if (message != DHCP_MESSAGE_OFFER) {
        return false;
    }

    if (result != NULL) {
        memcpy(&result->rent_time, rent_time_p, sizeof(*rent_time_p));
        memcpy(&result->server, server_p, sizeof(*server_p));
        memcpy(&result->mask, mask_p, sizeof(*mask_p));
        result->ip = bootp_frame->yiaddr;
    }

    return true;
}

void dhcp_request(
    struct mod_eth_frame *frame,
    uint32_t xid,
    struct dhcp_offer offer
) {
    frame_setup_ethernet(frame, BROADCAST_MAC, ETHERTYPE_IPV4);

    frame_setup_ipv4(frame, IPV4_PROTOCOL_UDP, ZERO_IP, BROADCAST_IP);

    frame_setup_bootp(
        frame,
        BOOTP_OP_REQUEST,
        xid,
        ZERO_IP,
        ZERO_IP,
        ZERO_IP,
        ZERO_IP,
        eth_mac()
    );

    dhcp_setup(frame);
    dhcp_add_option_message(frame, DHCP_MESSAGE_REQUEST);
    dhcp_add_option_request_ip(frame, offer.ip);
    dhcp_add_option_server(frame, offer.server);

    size_t options_size = dhcp_options_size(frame);
    size_t dhcp_size = sizeof(struct dhcp_frame) - sizeof(struct udp_frame) - 1;
    size_t size = dhcp_size + options_size;
    frame_setup_udp(frame, DHCP_PORT_CLIENT, DHCP_PORT_SERVER, size);

    frame->size = sizeof(struct dhcp_frame) - 1 + options_size;
}

bool dhcp_ack(
    struct mod_eth_frame *frame,
    uint32_t xid,
    struct dhcp_offer *result
) {
    if (frame->size < sizeof(struct dhcp_frame)) {
        return false;
    }

    struct ethernet_frame *ethernet_frame =
        (struct ethernet_frame *) frame->payload;
    struct ipv4_frame *ipv4_frame = (struct ipv4_frame *) frame->payload;
    struct bootp_frame *bootp_frame = (struct bootp_frame *) frame->payload;
    struct dhcp_frame *dhcp_frame = (struct dhcp_frame *) frame->payload;

    if (!mac_eq(ethernet_frame->dest_mac, eth_mac())
        || ethernet_frame->ethertype != ETHERTYPE_IPV4) {
        return false;
    }

    if (!frame_valid_ipv4(frame, IPV4_PROTOCOL_UDP)
        || !ip_eq(ipv4_frame->src_ip, result->server)
        || !ip_eq(ipv4_frame->dest_ip, result->ip)) {
        return false;
    }

    if (!frame_valid_udp(frame, true, DHCP_PORT_SERVER, true, DHCP_PORT_CLIENT)) {
        return false;
    }

    if (!frame_valid_bootp(frame) || !mac_eq(bootp_frame->chaddr.mac, eth_mac())
        || bootp_frame->op != BOOTP_OP_REPLY || bootp_frame->xid != xid
        || !ip_eq(bootp_frame->yiaddr, result->ip)) {
        return false;
    }

    if (dhcp_frame->magic != DHCP_MAGIC) {
        return false;
    }

    uint8_t *message_p = dhcp_get_option(frame, DHCP_OPTION_MESSAGE);
    uint32_t *rent_time_p = dhcp_get_option(frame, DHCP_OPTION_RENT_TIME);
    struct drv_udpsrv_ip *server_p = dhcp_get_option(frame, DHCP_OPTION_SERVER);
    struct drv_udpsrv_ip *mask_p = dhcp_get_option(frame, DHCP_OPTION_MASK);

    if (message_p == NULL || rent_time_p == NULL || server_p == NULL
        || mask_p == NULL) {
        return false;
    }

    uint8_t message;
    memcpy(&message, message_p, sizeof(*message_p));

    if (message != DHCP_MESSAGE_ACK) {
        return false;
    }

    if (result != NULL) {
        memcpy(&result->rent_time, rent_time_p, sizeof(*rent_time_p));
        memcpy(&result->server, server_p, sizeof(*server_p));
        memcpy(&result->mask, mask_p, sizeof(*mask_p));
        result->ip = bootp_frame->yiaddr;
    }

    return true;
}
