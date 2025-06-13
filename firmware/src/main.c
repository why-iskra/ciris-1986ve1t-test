//
// Created by whyiskra on 2025-05-24.
//

#include "drv/clock.h"
#include "drv/lcd.h"
#include "drv/terminal.h"
#include "drv/udpsrv.h"
#include "mod/clk.h"
#include "mod/port.h"
#include "mod/watchdog.h"
#include <stdio.h>

#define CMD_MAGIC       (0x236F304C)
#define CMD_QUEUE_SIZE  (32)
#define CMD_PORT        (30000)

#define LED1       MOD_PORT_D, 11
#define LED2       MOD_PORT_D, 12
#define LED3       MOD_PORT_D, 13
#define LED4       MOD_PORT_D, 14
#define BTN_RIGHT  MOD_PORT_E, 5
#define BTN_BACK   MOD_PORT_E, 11
#define BTN_BOTTOM MOD_PORT_E, 15
#define BTN_TOP    MOD_PORT_E, 8
#define BTN_SELECT MOD_PORT_E, 9
#define BTN_LEFT   MOD_PORT_E, 10

typedef enum {
    CMD_TYPE_LED = 0,
    CMD_TYPE_BTN = 1,
    CMD_TYPE_LCD_WRITE = 2,
    CMD_TYPE_LCD_SETCUR = 3,
    CMD_TYPE_LCD_CLEAR = 4,
} cmd_type_t;

struct cmd {
    uint32_t type;
    union {
        struct {
            uint8_t val1;
            uint8_t val2;
            uint8_t val3;
            uint8_t val4;
        } led;
        struct {
            uint8_t left;
            uint8_t right;
            uint8_t top;
            uint8_t bottom;
            uint8_t select;
            uint8_t back;
        } button;
        struct {
            char text[16];
        } lcd_write;
        struct {
            uint8_t x;
            uint8_t y;
        } lcd_setcur;
    };
    uint32_t magic;
} __attribute__((packed));

struct cmd_extended {
    struct cmd cmd;
    struct drv_udpsrv_ip ip;
};

static struct mod_eth_mac mac = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc };
static struct drv_udpsrv_ip ip = { 192, 168, 1, 164 };
static struct drv_udpsrv_ip mask = { 255, 255, 0, 0 };

static volatile uint8_t cmd_queue_head = 0;
static volatile uint8_t cmd_queue_tail = 0;
static struct cmd_extended cmd_queue[CMD_QUEUE_SIZE];

static void cmd_queue_push(struct cmd_extended cmd_extended) {
    uint8_t next = (cmd_queue_head + 1) % CMD_QUEUE_SIZE;
    if (next == cmd_queue_tail) {
        return;
    }

    cmd_queue[cmd_queue_head] = cmd_extended;
    cmd_queue_head = next;
}

static bool cmd_queue_pop(struct cmd_extended *cmd_extended) {
    if (cmd_queue_head == cmd_queue_tail) {
        return false;
    }

    if (cmd_extended != NULL) {
        *cmd_extended = cmd_queue[cmd_queue_tail];
    }
    cmd_queue_tail = (cmd_queue_tail + 1) % CMD_QUEUE_SIZE;

    return true;
}

static void handler(const void *data, size_t size, struct udp_info info) {
    if (info.dest_port != CMD_PORT || size != sizeof(struct cmd)) {
        return;
    }

    const struct cmd *cmd = data;
    if (cmd->magic != CMD_MAGIC) {
        return;
    }

    struct cmd_extended extended = {
        .cmd = *cmd,
        .ip = info.src_ip,
    };

    cmd_queue_push(extended);
}

static bool read_button(mod_port_t port, int pin) {
    uint8_t acc = 0;
    for (int i = 0; i < 32; i++) {
        if (port_read(port, pin)) {
            acc++;
        }

        clock_delay_us(100);
    }

    return acc > 24;
}

static void handle_cmd(struct cmd_extended extended) {
    struct cmd cmd = extended.cmd;
    switch ((cmd_type_t) cmd.type) {
        case CMD_TYPE_LED: {
            port_write(LED1, cmd.led.val1 > 0);
            port_write(LED2, cmd.led.val2 > 0);
            port_write(LED3, cmd.led.val3 > 0);
            port_write(LED4, cmd.led.val4 > 0);
            break;
        }
        case CMD_TYPE_BTN: {
            struct cmd response = {
                .type = CMD_TYPE_BTN,
                .button.left = read_button(BTN_LEFT) ? 0xff : 0x00,
                .button.right = read_button(BTN_RIGHT) ? 0xff : 0x00,
                .button.top = read_button(BTN_TOP) ? 0xff : 0x00,
                .button.bottom = read_button(BTN_BOTTOM) ? 0xff : 0x00,
                .button.select = read_button(BTN_SELECT) ? 0xff : 0x00,
                .button.back = read_button(BTN_BACK) ? 0xff : 0x00,
                .magic = CMD_MAGIC,
            };

            udpsrv_send(extended.ip, 30000, 30000, &response, sizeof(response));
            break;
        }
        case CMD_TYPE_LCD_WRITE: {
            for (size_t i = 0; i < sizeof(cmd.lcd_write.text); i++) {
                char c = cmd.lcd_write.text[i];
                if (c == 0) {
                    break;
                }

                terminal_printc(c);
            }
            break;
        }
        case CMD_TYPE_LCD_SETCUR: {
            terminal_set_cursor_x(cmd.lcd_setcur.x);
            terminal_set_cursor_y(cmd.lcd_setcur.y);
            break;
        }
        case CMD_TYPE_LCD_CLEAR: {
            terminal_clear();
            break;
        }
    }
}

static void setup_peripheral(void) {
    clk_en_peripheral(MOD_CLK_PERIPHERAL_PORTD, true);
    clk_en_peripheral(MOD_CLK_PERIPHERAL_PORTE, true);

    struct mod_port_cfg cfg_led = port_default_cfg();
    cfg_led.func = MOD_PORT_FUNC_PORT;
    cfg_led.mode = MOD_PORT_MODE_DIGITAL;
    cfg_led.dir = MOD_PORT_DIR_OUT;
    cfg_led.speed = MOD_PORT_SPEED_FAST;

    struct mod_port_cfg cfg_button = port_default_cfg();
    cfg_button.func = MOD_PORT_FUNC_PORT;
    cfg_button.mode = MOD_PORT_MODE_DIGITAL;
    cfg_button.dir = MOD_PORT_DIR_IN;
    cfg_button.speed = MOD_PORT_SPEED_SLOW;

    port_setup(LED1, cfg_led);
    port_setup(LED2, cfg_led);
    port_setup(LED3, cfg_led);
    port_setup(LED4, cfg_led);

    port_setup(BTN_LEFT, cfg_button);
    port_setup(BTN_RIGHT, cfg_button);
    port_setup(BTN_TOP, cfg_button);
    port_setup(BTN_BOTTOM, cfg_button);
    port_setup(BTN_SELECT, cfg_button);
    port_setup(BTN_BACK, cfg_button);

    port_write(LED1, false);
    port_write(LED2, false);
    port_write(LED3, false);
    port_write(LED4, false);
}

int main(void) {
    clk_set_cpu_freq(MOD_CLK_CPU_FREQ_96MHZ);

    setup_peripheral();

    drv_clock_init();
    drv_lcd_init();
    drv_terminal_init();

    lcd_fill(0xff);
    drv_udpsrv_init(mac, ip, mask, handler);
    lcd_fill(0x00);

    uint32_t ticks = 0;
    while (true) {
        struct cmd_extended extended;
        if (cmd_queue_pop(&extended)) {
            handle_cmd(extended);
        }

        if ((ticks++) % 100000 == 0) {
            terminal_update(true);
        }

        iwdt_notify();
    }
}
