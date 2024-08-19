#pragma once

#include "types.h"
#include "editor.h"

typedef struct cursor_s {
  i32 x, y;
  i32 coloff, rowoff;
  u16 max_col, max_row;
} cursor_t;


char cursor_char(editor_t* E);

void cursor_bol(editor_t* E);

void cursor_eol(editor_t* E);

void cursor_page_up(editor_t* E);

void cursor_page_down(editor_t* E);

void cursor_move(editor_t* E, i32 x, i32 y);

void cursor_move_word_forward(editor_t* E);

void cursor_move_word_backward(editor_t* E);

void cursor_remove_char(editor_t* E);

void cursor_insert_char(editor_t* E, int c);

void cursor_insert_text(editor_t* E, const char* text);

void cursor_set_max(u16 max_col, u16 max_row);

void cursor_down(editor_t* E);

void cursor_up(editor_t* E);
