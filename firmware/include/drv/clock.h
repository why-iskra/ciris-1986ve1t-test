//
// Created by whyiskra on 2025-05-28.
//

#pragma once

#include <stdint.h>

void drv_clock_init(void);

void clock_delay_us(uint8_t);
void clock_delay_ms(uint32_t);
uint32_t clock_millis(void);
