#pragma once

#include "definitions.h"

void cursor_init(cursor_t *C);

void cursor_set(buffer_t *dest, cursor_t *src);

void cursor_update_window_size(buffer_t *B, u16 rows, u16 cols);

void cursor_region_text(buffer_t *B);

void cursor_region_kill(buffer_t *B);

char cursor_char(buffer_t *B);

void cursor_bol(buffer_t *B);

void cursor_eol(buffer_t *B);

void cursor_eof(buffer_t *B);

void cursor_bof(buffer_t *B);

void cursor_page_up(buffer_t *B);

void cursor_page_down(buffer_t *B);

void cursor_move_paragraph_forward(buffer_t *B);

void cursor_move_paragraph_backward(buffer_t *B);

void cursor_move_word_forward(buffer_t *B);

void cursor_move_word_backward(buffer_t *B);

void cursor_move_line_up(buffer_t *B);

void cursor_move_region_forward(buffer_t *B);

void cursor_move_region_backward(buffer_t *B);

void cursor_remove_char(buffer_t *B);

void cursor_insert_char(buffer_t *B, i32 c);

void cursor_insert_text(buffer_t *B, const char* text);

void cursor_down(buffer_t *B);

void cursor_up(buffer_t *B);

void cursor_right(buffer_t *B);

void cursor_left(buffer_t *B);

void cursor_break_line(buffer_t *B);

void cursor_delete_forward(buffer_t *B);

void cursor_delete_row(buffer_t *B);

void cursor_paste(buffer_t *B);

void cursor_undo(buffer_t *B);

void cursor_goto(buffer_t *B, u32 x, u32 y);

void cursor_free(cursor_t *C);

void cursor_recenter(buffer_t *B);
