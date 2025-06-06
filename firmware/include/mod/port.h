//
// Created by whyiskra on 2025-05-24.
//

#pragma once

#include <stdbool.h>

typedef enum {
    MOD_PORT_A,
    MOD_PORT_B,
    MOD_PORT_C,
    MOD_PORT_D,
    MOD_PORT_E,
    MOD_PORT_F,
} mod_port_t;

typedef enum {
    MOD_PORT_DIR_IN = 0,
    MOD_PORT_DIR_OUT = 1,
} mod_port_dir_t;

typedef enum {
    MOD_PORT_FUNC_PORT = 0x0,
    MOD_PORT_FUNC_MAIN = 0x1,
    MOD_PORT_FUNC_ALT = 0x2,
    MOD_PORT_FUNC_OVERRIDE = 0x3,
} mod_port_func_t;

typedef enum {
    MOD_PORT_MODE_ANALOG = 0,
    MOD_PORT_MODE_DIGITAL = 1,
} mod_port_mode_t;

typedef enum {
    MOD_PORT_PULL_OFF = 0,
    MOD_PORT_PULL_ON = 1,
} mod_port_pull_t;

typedef enum {
    MOD_PORT_SCHMIDT_OFF = 0,
    MOD_PORT_SCHMIDT_ON = 1,
} mod_port_schmidt_t;

typedef enum {
    MOD_PORT_PD_DRIVER = 0,
    MOD_PORT_PD_OPEN = 1,
} mod_port_pd_t;

typedef enum {
    MOD_PORT_SPEED_OFF = 0x0,
    MOD_PORT_SPEED_SLOW = 0x1,
    MOD_PORT_SPEED_MEDIUM = 0x2,
    MOD_PORT_SPEED_FAST = 0x3,
} mod_port_speed_t;

typedef enum {
    MOD_PORT_GFEN_OFF = 0,
    MOD_PORT_GFEN_ON = 1,
} mod_port_gfen_t;

struct mod_port_cfg {
    mod_port_dir_t dir;
    mod_port_func_t func;
    mod_port_mode_t mode;
    mod_port_pull_t pull_up;
    mod_port_pull_t pull_down;
    mod_port_schmidt_t schmidt;
    mod_port_gfen_t gfen;
    mod_port_pd_t pd;
    mod_port_speed_t speed;
};

#define port_default_cfg() ((struct mod_port_cfg) { \
        .dir = MOD_PORT_DIR_IN, \
        .func = MOD_PORT_FUNC_PORT, \
        .mode = MOD_PORT_MODE_ANALOG, \
        .pull_up = MOD_PORT_PULL_OFF, \
        .pull_down = MOD_PORT_PULL_OFF, \
        .schmidt = MOD_PORT_SCHMIDT_OFF, \
        .gfen = MOD_PORT_GFEN_OFF, \
        .pd = MOD_PORT_PD_DRIVER, \
        .speed = MOD_PORT_SPEED_OFF, \
    })

int port_setup(mod_port_t, int, struct mod_port_cfg);
int port_reset(mod_port_t);
int port_write(mod_port_t, int, bool);
int port_read(mod_port_t, int);
