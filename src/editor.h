#pragma once

#include "types.h"

#define VERSION "0.0.1"

line_t *lalloc(usize capacity);

line_t* line_append_s(line_t *lp, const char *str, usize len);

line_t* line_append(line_t *lp, const char *str);

void line_free(line_t *lp);

line_t *editor_move_line_up(editor_t *E, line_t *lp);

void editor_delete_forward(line_t *lp, i32 x);

void editor_delete_backward(line_t *lp, i32 x);

void editor_delete_lines(editor_t *E, line_t* lp, i32 size);

void editor_break_line(editor_t *E, line_t *lp, i32 x);

void editor_delete_char_at(line_t *lp, i32 x);

line_t* editor_insert_char_at(editor_t *E, line_t *lp, i32 x, char ch);

void editor_text_between(editor_t *E, mark_t mark, char *r);

void editor_kill_between(editor_t *E, mark_t mark, char *r);

char editor_char_at(line_t *lp, i32 x);

line_t* editor_insert_row_at(editor_t* E, usize n);

line_t* editor_insert_row_with_data_at(editor_t *E, usize y, char* strdata);

void editor_insert_text(editor_t *E, line_t* lp, i32 x, const char* str);

line_t* editor_rows_to_string(line_t* lines, unsigned int size);

editor_t editor_init(const char* filename);

