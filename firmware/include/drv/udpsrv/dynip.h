//
// Created by whyiskra on 2025-06-12.
//

#pragma once

#include "drv/udpsrv.h"
#include <stdbool.h>

void dynip_init(void);
bool dynip_has(void);
struct drv_udpsrv_ip dynip_get(void);
void dynip_set(struct drv_udpsrv_ip, uint32_t);
void dynip_validate(void);
