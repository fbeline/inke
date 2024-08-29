#pragma once

#include "types.h"
#include "editor.h"

typedef struct region_s {
  bool active;
  vec2_t pos;    // raw position
  vec2_t vpos;  //  virtual position
} region_t;

typedef struct cursor_s {
  i32 x, y;
  region_t region;
  i32 coloff, rowoff;
  u16 max_col, max_row;
} cursor_t;

cursor_t cursor_get(void);

void cursor_set(cursor_t* cursor);

vec2_t cursor_position(void);

region_t cursor_region(void);

void cursor_region_start(void);

char* cursor_region_text(editor_t* E);

char* cursor_region_kill(editor_t* E);

void cursor_clear_region(void);

char cursor_char(editor_t* E);

void cursor_bol();

void cursor_eol(editor_t* E);

void cursor_eof(editor_t* E);

void cursor_bof();

void cursor_page_up(editor_t* E);

void cursor_page_down(editor_t* E);

void cursor_move_word_forward(editor_t* E);

void cursor_move_word_backward(editor_t* E);

void cursor_remove_char(editor_t* E);

void cursor_insert_char(editor_t* E, int c);

void cursor_insert_text(editor_t* E, char* text);

void cursor_set_max(u16 max_col, u16 max_row);

void cursor_down(editor_t* E);

void cursor_up(editor_t* E);

void cursor_right(editor_t* E);

void cursor_left(editor_t* E);

void cursor_break_line(editor_t* E);

void cursor_delete_forward(editor_t* E);

void cursor_delete_row(editor_t* E);
