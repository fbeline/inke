#pragma once

#include "utils.h"

void tt_flush(void);

void tt_puts(char *buf);

void vt_set_cursor_position(i32 x, i32 y);

void vt_show_cursor(void);

void vt_hide_cursor(void);

void vt_erase_display(void);

void vt_erase_line(void);

void vt_clear_screen(void);

void vt_cursor_forward(i32 pn);

void vt_cursor_down(i32 pn);

i32 vt_cursor_position(int *rows, int *cols);
