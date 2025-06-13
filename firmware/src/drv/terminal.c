//
// Created by whyiskra on 2025-05-28.
//

#include "drv/terminal.h"
#include "drv/lcd.h"
#include "font/6x8.h"
#include "mod/clk.h"
#include "mod/port.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#define INDICATE_PIN MOD_PORT_D, 7

#define TEXT_BUFFER_WIDTH  (21)
#define TEXT_BUFFER_HEIGHT (8)

static bool terminal_inited = false;
static int pos_x = 0;
static int pos_y = 0;

static char text_buffer[TEXT_BUFFER_HEIGHT][TEXT_BUFFER_WIDTH];
static char text_buffer_prev[TEXT_BUFFER_HEIGHT][TEXT_BUFFER_WIDTH];

int _write(int, const void *, size_t);
int _write(int fd, const void *buf, size_t count) {
    if (fd != 1 && fd != 2) {
        errno = EBADF;
        return -1;
    }

    if (!terminal_inited) {
        return 0;
    }

    const char *cbuf = (const char *) buf;
    for (size_t i = 0; i < count; ++i) {
        terminal_printc(cbuf[i]);
    }

    return count > INT_MAX ? INT_MAX : (int) count;
}

void drv_terminal_init(void) {
    struct mod_port_cfg cfg = port_default_cfg();
    cfg.dir = MOD_PORT_DIR_OUT;
    cfg.func = MOD_PORT_FUNC_PORT;
    cfg.mode = MOD_PORT_MODE_DIGITAL;
    cfg.speed = MOD_PORT_SPEED_FAST;

    clk_en_peripheral(MOD_CLK_PERIPHERAL_PORTD, true);
    port_setup(INDICATE_PIN, cfg);
    port_write(INDICATE_PIN, false);

    terminal_clear();

    terminal_inited = true;
}

void terminal_clear(void) {
    memset(text_buffer, 0, TEXT_BUFFER_WIDTH * TEXT_BUFFER_HEIGHT);
    memset(text_buffer_prev, 0, TEXT_BUFFER_WIDTH * TEXT_BUFFER_HEIGHT);
    pos_x = 0;
    pos_y = 0;

    lcd_fill(0);
}

int terminal_set_cursor_x(int value) {
    if (value < 0 || value >= TEXT_BUFFER_WIDTH) {
        return -1;
    }

    pos_x = value;
    return 0;
}

int terminal_set_cursor_y(int value) {
    if (value < 0 || value >= TEXT_BUFFER_HEIGHT) {
        return -1;
    }

    pos_y = value;
    return 0;
}

static void draw_char(int x, int y, char c) {
    port_write(INDICATE_PIN, true);

    uint8_t *font_slice = font_6x8 + (8 * (int) c);

    lcd_setpage((uint8_t) (TEXT_BUFFER_HEIGHT - 1 - y), false);
    lcd_setpage((uint8_t) (TEXT_BUFFER_HEIGHT - 1 - y), true);

    for (int i = 0; i < 6; i++) {
        uint8_t line = 0;
        for (int k = 0; k < 8; k++) {
            line |= ((font_slice[k] >> (i + 2)) & 0x1) << (7 - k);
        }

        lcd_writebyte(
            (uint8_t) (2 + ((TEXT_BUFFER_WIDTH - 1 - x) * 6) + i),
            line
        );
    }

    port_write(INDICATE_PIN, false);
}

static void move_up_lines(void) {
    for (int i = 1; i < TEXT_BUFFER_HEIGHT; i++) {
        memcpy(text_buffer[i - 1], text_buffer[i], TEXT_BUFFER_WIDTH);
    }
    memset(text_buffer[TEXT_BUFFER_HEIGHT - 1], 0, TEXT_BUFFER_WIDTH);
}

static void newline(void) {
    if (pos_y < TEXT_BUFFER_HEIGHT - 1) {
        pos_y++;
        return;
    }

    move_up_lines();
}

static void move_right(void) {
    pos_x++;
    if (pos_x != TEXT_BUFFER_WIDTH) {
        return;
    }

    pos_x = 0;
    newline();
}

static void del(void) {
    if (pos_x > 0) {
        pos_x--;
    } else if (pos_y > 0) {
        pos_x = TEXT_BUFFER_WIDTH - 1;
        pos_y--;
    }

    text_buffer[pos_y][pos_x] = ' ';
}

void terminal_printc(char c) {
    if (isprint((int) c)) {
        text_buffer[pos_y][pos_x] = c;
        move_right();
    } else {
        switch (c) {
            case '\n': {
                pos_x = 0;
                newline();
                break;
            }
            case '\r': {
                pos_x = 0;
                break;
            }
            case 127: {
                del();
                break;
            }
            default: {
                text_buffer[pos_y][pos_x] = ' ';
                move_right();
            }
        }
    }
}

void terminal_update(bool lazy) {
    for (int y = 0; y < TEXT_BUFFER_HEIGHT; y++) {
        for (int x = 0; x < TEXT_BUFFER_WIDTH; x++) {
            char c = text_buffer[y][x];
            if (!lazy || c != text_buffer_prev[y][x]) {
                draw_char(x, y, c);
                text_buffer_prev[y][x] = c;
            }
        }
    }
}
