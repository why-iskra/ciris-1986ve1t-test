//
// Created by whyiskra on 2025-05-28.
//

#include "drv/lcd.h"
#include "mod/clk.h"
#include "mod/extbus.h"
#include "mod/port.h"
#include "mod/asm.h"

#define LCD_CMD1 (*((uint32_t *) 0x68000000))
#define LCD_CMD2 (*((uint32_t *) 0x70000000))

#define LCD_DATA1 (*((uint32_t *) 0x6C000000))
#define LCD_DATA2 (*((uint32_t *) 0x74000000))

static void cmd_delay(void) {
    for (int i = 0; i < 64; i ++) {
        asm_nop();
    }
}

void lcd_setstartline(uint8_t line) {
    LCD_CMD1 = 0xC0 | (line & 0x3F);
    cmd_delay();

    LCD_CMD2 = 0xC0 | (line & 0x3F);
    cmd_delay();
}

void lcd_onoff(bool on) {
    uint32_t command = on ? 0x3F : 0x3E;

    LCD_CMD1 = command;
    cmd_delay();

    LCD_CMD2 = command;
    cmd_delay();
}

void lcd_setpage(uint8_t page, bool second_crystal) {
    if (second_crystal) {
        LCD_CMD2 = 0xB8 | (page & 0x07);
    } else {
        LCD_CMD1 = 0xB8 | (page & 0x07);
    }

    cmd_delay();
}

void lcd_setaddress(uint8_t address) {
    if (address < 64) {
        LCD_CMD1 = 0x40 | (address & 0x3F);
    } else {
        LCD_CMD2 = 0x40 | (address & 0x3F);
    }

    cmd_delay();
}

uint32_t lcd_readstatus(bool second_crystal) {
    uint32_t result;
    if (second_crystal) {
        result = LCD_CMD2;
    } else {
        result = LCD_CMD1;
    }

    cmd_delay();

    return result;
}

void lcd_writebyte(uint8_t address, uint8_t byte) {
    lcd_setaddress(address);

    if (address < 64) {
        LCD_DATA1 = byte;
    } else {
        LCD_DATA2 = byte;
    }

    cmd_delay();
}

uint8_t lcd_readbyte(uint8_t address) {
    lcd_setaddress(address);

    uint8_t result;
    if (address < 64) {
        result = (uint8_t) LCD_DATA1;
        cmd_delay();
        result = (uint8_t) LCD_DATA1;
    } else {
        result = (uint8_t) LCD_DATA2;
        cmd_delay();
        result = (uint8_t) LCD_DATA2;
    }

    cmd_delay();

    return result;
}

void lcd_fill(uint8_t b) {
    lcd_setaddress(0);
    for (uint8_t j = 0; j < 8; j++) {
        lcd_setpage(j, false);
        for (uint8_t i = 0; i < 64; i++) {
            lcd_writebyte(i, b);
        }
    }
    lcd_setaddress(64);
    for (uint8_t j = 0; j < 8; j++) {
        lcd_setpage(j, true);
        for (uint8_t i = 64; i < 128; i++) {
            lcd_writebyte(i, b);
        }
    }
    lcd_setaddress(0);
    lcd_setpage(0, true);
    lcd_setaddress(64);
    lcd_setpage(0, false);
}

void drv_lcd_init(void) {
    {
        clk_en_peripheral(MOD_CLK_PERIPHERAL_PORTA, true);
        clk_en_peripheral(MOD_CLK_PERIPHERAL_PORTC, true);
        clk_en_peripheral(MOD_CLK_PERIPHERAL_PORTE, true);

        struct mod_port_cfg port_bus_cfg = port_default_cfg();
        port_bus_cfg.func = MOD_PORT_FUNC_MAIN;
        port_bus_cfg.mode = MOD_PORT_MODE_DIGITAL;
        port_bus_cfg.speed = MOD_PORT_SPEED_FAST;

        for (int i = 0; i < 8; i++) {
            port_setup(MOD_PORT_A, i, port_bus_cfg);
        }

        port_setup(MOD_PORT_C, 0, port_bus_cfg);

        port_bus_cfg.func = MOD_PORT_FUNC_ALT;
        port_setup(MOD_PORT_C, 2, port_bus_cfg);

        port_setup(MOD_PORT_E, 14, port_bus_cfg);
        port_setup(MOD_PORT_E, 13, port_bus_cfg);
        port_setup(MOD_PORT_E, 12, port_bus_cfg);
    }

    {
        clk_en_peripheral(MOD_CLK_PERIPHERAL_EXT_BUS_CNTRL, true);

        struct mod_extbus_cfg cfg = extbus_default_cfg();
        cfg.mode = MOD_EXTBUS_RAM;
        extbus_setup(cfg);
    }

    lcd_fill(0x00);
    lcd_onoff(true);
    lcd_setstartline(0);
}
