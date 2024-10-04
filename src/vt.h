#pragma once

#include "types.h"

void vt_flush(void);

void vt_puts(const char *str);

void vt_fputs(const char *str, ...);

void vt_nputs(const char *str, usize n);

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
