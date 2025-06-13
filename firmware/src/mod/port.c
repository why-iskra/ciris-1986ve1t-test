//
// Created by whyiskra on 2025-05-24.
//

#include "mod/port.h"
#include "mod/regs.h"
#include "utils/bits.h"
#include <stddef.h>

#define PIN_COUNT (16)

static inline int validate_dir(mod_port_dir_t value) {
    switch (value) {
        case MOD_PORT_DIR_IN:
        case MOD_PORT_DIR_OUT: return 0;
    }

    return -1;
}

static inline int validate_func(mod_port_func_t value) {
    switch (value) {
        case MOD_PORT_FUNC_PORT:
        case MOD_PORT_FUNC_MAIN:
        case MOD_PORT_FUNC_ALT:
        case MOD_PORT_FUNC_OVERRIDE: return 0;
    }

    return -1;
}

static inline int validate_mode(mod_port_mode_t value) {
    switch (value) {
        case MOD_PORT_MODE_ANALOG:
        case MOD_PORT_MODE_DIGITAL: return 0;
    }

    return -1;
}

static inline int validate_pull(mod_port_pull_t value) {
    switch (value) {
        case MOD_PORT_PULL_OFF:
        case MOD_PORT_PULL_ON: return 0;
    }

    return -1;
}

static inline int validate_schmidt(mod_port_schmidt_t value) {
    switch (value) {
        case MOD_PORT_SCHMIDT_OFF:
        case MOD_PORT_SCHMIDT_ON: return 0;
    }

    return -1;
}

static inline int validate_pd(mod_port_pd_t value) {
    switch (value) {
        case MOD_PORT_PD_DRIVER:
        case MOD_PORT_PD_OPEN: return 0;
    }

    return -1;
}

static inline int validate_speed(mod_port_speed_t value) {
    switch (value) {
        case MOD_PORT_SPEED_OFF:
        case MOD_PORT_SPEED_SLOW:
        case MOD_PORT_SPEED_MEDIUM:
        case MOD_PORT_SPEED_FAST: return 0;
    }

    return -1;
}

static inline int validate_gfen(mod_port_gfen_t value) {
    switch (value) {
        case MOD_PORT_GFEN_OFF:
        case MOD_PORT_GFEN_ON: return 0;
    }

    return -1;
}

static inline int validate_cfg(struct mod_port_cfg cfg) {
#define vld(n) validate_##n(cfg.n)
    if (vld(mode) || vld(func) || vld(dir) || vld(gfen) || vld(schmidt)
        || vld(speed) || vld(pd)) {
        return -1;
    }
#undef vld

    if (validate_pull(cfg.pull_up) || validate_pull(cfg.pull_down)) {
        return -1;
    }

    return 0;
}

static inline struct port_regs *get_port(mod_port_t port) {
    switch (port) {
        case MOD_PORT_A: return get_port_regs(A);
        case MOD_PORT_B: return get_port_regs(B);
        case MOD_PORT_C: return get_port_regs(C);
        case MOD_PORT_D: return get_port_regs(D);
        case MOD_PORT_E: return get_port_regs(E);
        case MOD_PORT_F: return get_port_regs(F);
    }

    return NULL;
}

int port_setup(mod_port_t port, int pin, struct mod_port_cfg cfg) {
    struct port_regs *port_regs = get_port(port);
    if (port_regs == NULL || validate_cfg(cfg) || pin < 0 || pin >= PIN_COUNT) {
        return -1;
    }

    struct port_regs stamp = *port_regs;

    set_bit(stamp.oe, pin, cfg.dir);
    set_bits(stamp.func, pin * 2, 0x3, cfg.func);
    set_bit(stamp.analog, pin, cfg.mode);
    set_bit(stamp.pull, pin, cfg.pull_down);
    set_bit(stamp.pull, pin + 16, cfg.pull_up);
    set_bit(stamp.gfen, pin, cfg.gfen);
    set_bit(stamp.pd, pin, cfg.pd);
    set_bit(stamp.pd, pin + 16, cfg.schmidt);
    set_bits(stamp.pwr, pin * 2, 0x3, cfg.speed);

    port_regs->oe = stamp.oe;
    port_regs->func = stamp.func;
    port_regs->analog = stamp.analog;
    port_regs->pull = stamp.pull;
    port_regs->pd = stamp.pd;
    port_regs->pwr = stamp.pwr;
    port_regs->gfen = stamp.gfen;

    return 0;
}

int port_reset(mod_port_t port) {
    struct port_regs *port_regs = get_port(port);
    if (port_regs == 0) {
        return -1;
    }

    for (int i = 0; i < PIN_COUNT; i++) {
        if (port_setup(port, i, port_default_cfg())) {
            return -1;
        }
    }

    return 0;
}

int port_write(mod_port_t port, int pin, bool value) {
    struct port_regs *port_regs = get_port(port);
    if (port_regs == 0) {
        return -1;
    }

    if (pin < 0 || pin >= PIN_COUNT) {
        return -1;
    }

    set_bit(port_regs->rxtx, pin, value ? 1 : 0);
    return 0;
}

int port_read(mod_port_t port, int pin) {
    struct port_regs *port_regs = get_port(port);
    if (port_regs == 0) {
        return -1;
    }

    if (pin < 0 || pin >= PIN_COUNT) {
        return -1;
    }

    return get_bit(port_regs->rxtx, pin);
}
