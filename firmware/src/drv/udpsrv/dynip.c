//
// Created by whyiskra on 2025-06-12.
//

#include "drv/udpsrv/dynip.h"
#include "drv/clock.h"

static bool has_ip;
static struct drv_udpsrv_ip ip;
static uint32_t timestamp;
static uint32_t rent_ms;

void dynip_init(void) {
    has_ip = false;
    ip = ZERO_IP;
    timestamp = 0;
    rent_ms = 0;
}

bool dynip_has(void) {
    return has_ip;
}

struct drv_udpsrv_ip dynip_get(void) {
    return has_ip ? ip : ZERO_IP;
}

void dynip_set(struct drv_udpsrv_ip new_ip, uint32_t rent_sec) {
    ip = new_ip;
    rent_ms = rent_sec * 1000;
    timestamp = clock_millis();
    has_ip = true;
}

void dynip_validate(void) {
    if (!has_ip) {
        return;
    }

    if (clock_millis() - timestamp > rent_ms) {
        has_ip = false;
    }
}
