#pragma once

#include "definitions.h"

#define C_BLACK         30
#define C_RED           31
#define C_GREEN         32
#define C_YELLOW        33
#define C_BLUE          34
#define C_MAGENTA       35
#define C_CYAN          36
#define C_WHITE         37

#define C_RESET         0
#define C_BOLD          1
#define C_UNDERLINE     4
#define C_REVERSED      7

void vt_flush(void);

void vt_puts(const char *str);

void vt_nputs(const char *str, usize n);

void vt_fputs(const char *str, ...);

void vt_puts_color(const char *str, u16 color);

void vt_set_cursor_position(i32 x, i32 y);

void vt_show_cursor(void);

void vt_hide_cursor(void);

void vt_erase_display(void);

void vt_erase_line(void);

void vt_clear_screen(void);

void vt_cursor_forward(i32 pn);

void vt_cursor_down(i32 pn);

void vt_reset_text_attr(void);

void vt_reverse_video(void);

void vt_init(void);
