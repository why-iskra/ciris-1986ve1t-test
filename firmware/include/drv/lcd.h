//
// Created by whyiskra on 2025-05-28.
//

#pragma once

#include <stdbool.h>
#include <stdint.h>

void drv_lcd_init(void);

void lcd_setstartline(uint8_t line);
void lcd_onoff(bool on);
void lcd_setpage(uint8_t page, bool second_crystal);
void lcd_setaddress(uint8_t address);
uint32_t lcd_readstatus(bool second_crystal);
void lcd_writebyte(uint8_t address, uint8_t byte);
uint8_t lcd_readbyte(uint8_t address);
void lcd_fill(uint8_t b);
