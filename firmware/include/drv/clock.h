//
// Created by whyiskra on 2025-05-28.
//

#pragma once

#include <stdint.h>

typedef enum {
    DRV_CLOCK_UNIT_MS,    
    DRV_CLOCK_UNIT_US,    
} drv_clock_unit_t;

void drv_clock_init(void);

int clock_delay(uint32_t, drv_clock_unit_t);
