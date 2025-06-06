//
// Created by whyiskra on 2025-05-28.
//

#pragma once

#include <stddef.h>

void drv_terminal_init(void);

void terminal_clear(void);
int terminal_set_cursor_x(int);
int terminal_set_cursor_y(int);
void terminal_printc(char);
