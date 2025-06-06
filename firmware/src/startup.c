//
// Created by whyiskra on 2025-05-27.
//

#include "mod/asm.h"
#include "mod/clk.h"
#include "mod/port.h"
#include "mod/reset.h"
#include "mod/watchdog.h"
#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#define get_point(t, n) ((t *) &__##n##__)

extern uint32_t __text_end__;
extern uint32_t __data_start__;
extern uint32_t __data_end__;
extern uint32_t __bss_start__;
extern uint32_t __bss_end__;
extern uint64_t __heap_start__;
extern uint64_t __heap_end__;

static uintptr_t heap_brk = (uintptr_t) get_point(uint64_t, heap_start);

extern int main(void);
void __start(void);

void __isr_default(void);
void __isr_default(void) {
    software_reset();
    while (true) { }
}

static inline uintptr_t align8(uintptr_t value) {
    return (value + 7) & ~(uintptr_t) 7;
}

void *_sbrk(ptrdiff_t);
void *_sbrk(ptrdiff_t increment) {
    if (increment == 0) {
        return (void *) heap_brk;
    }

    uintptr_t aligned = align8((uintptr_t) increment);

    uintptr_t new_brk;
    if (increment > 0) {
        if (heap_brk > SIZE_MAX - aligned) {
            errno = ENOMEM;
            return (void *) -1;
        }

        new_brk = heap_brk + aligned;
        if (new_brk > (uintptr_t) get_point(uint64_t, heap_end)) {
            errno = ENOMEM;
            return (void *) -1;
        }
    } else {
        if (aligned > heap_brk) {
            errno = ENOMEM;
            return (void *) -1;
        }

        new_brk = heap_brk - aligned;
        if (new_brk < (uintptr_t) get_point(uint64_t, heap_start)) {
            errno = ENOMEM;
            return (void *) -1;
        }
    }

    void *old_brk = (void *) heap_brk;
    heap_brk = new_brk;
    return old_brk;
}

static void section_data_init(void) {
    uint32_t *from = get_point(uint32_t, text_end);
    uint32_t *to = get_point(uint32_t, data_start);

    for (; to < get_point(uint32_t, data_end); from++, to++) {
        *to = *from;
    }
}

static void section_bss_init(void) {
    uint32_t *addr = get_point(uint32_t, bss_start);
    uint32_t *to = get_point(uint32_t, bss_end);

    for (; addr < to; addr++) {
        *addr = 0;
    }
}

static void nop_delay(int ticks) {
    for (int i = 0; i < ticks; i++) {
        asm_nop();
    }
}

static void indicate_startup(void) {
    struct mod_port_cfg cfg = port_default_cfg();
    cfg.dir = MOD_PORT_DIR_OUT;
    cfg.mode = MOD_PORT_MODE_DIGITAL;
    cfg.speed = MOD_PORT_SPEED_FAST;

    clk_en_peripheral(MOD_CLK_PERIPHERAL_PORTD, true);
    port_setup(MOD_PORT_D, 7, cfg);

    for (int i = 0; i < 2; i++) {
        port_write(MOD_PORT_D, 7, 1);
        nop_delay(50000);
        port_write(MOD_PORT_D, 7, 0);
        nop_delay(50000);
    }

    port_write(MOD_PORT_D, 7, 1);
    nop_delay(200000);
    port_write(MOD_PORT_D, 7, 0);

    port_setup(MOD_PORT_D, 7, port_default_cfg());
    clk_en_peripheral(MOD_CLK_PERIPHERAL_PORTD, false);
}

void __start(void) {
    // FIXME: watchdog hardware bug (no reset counter) or wrong documentation
    // (?) watchdog temporary disabled
    //    clk_en_peripheral(MOD_CLK_PERIPHERAL_IWDT, true);

    section_bss_init();
    section_data_init();

    clk_init();
    indicate_startup();
    iwdt_setup(MOD_IWDG_FREQ_DIV_256, 0x1FF);

    asm_enable_irq();

    main();

    while (1) { }
}
