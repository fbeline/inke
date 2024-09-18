#pragma once

#include "types.h"
#include "editor.h"

struct cursor_s;

typedef struct region_s {
  bool active;
  struct cursor_s *cursor;
  line_t *lp;
  i32 offset, size;
} region_t;

typedef struct cursor_s {
  i32 x, y;
  region_t region;
  i32 coloff, rowoff;
  u32 max_col, max_row;
  editor_t* editor;
  line_t* clp;
} cursor_t;

cursor_t cursor_init(editor_t* E);

void cursor_set(cursor_t* dest, cursor_t* src);

vec2_t cursor_position(cursor_t* cursor);

void cursor_region_start(cursor_t* cursor);

char* cursor_region_text(cursor_t* cursor);

char* cursor_region_kill(cursor_t* cursor);

void cursor_clear_region(cursor_t* cursor);

char cursor_char(cursor_t* cursor);

void cursor_bol(cursor_t* cursor);

void cursor_eol(cursor_t* cursor);

void cursor_eof(cursor_t* cursor);

void cursor_bof(cursor_t* cursor);

void cursor_page_up(cursor_t* cursor);

void cursor_page_down(cursor_t* cursor);

void cursor_move_word_forward(cursor_t* cursor);

void cursor_move_word_backward(cursor_t* cursor);

void cursor_move_line_up(cursor_t *C);

void cursor_remove_char(cursor_t* cursor);

void cursor_insert_char(cursor_t* cursor, int c);

void cursor_insert_text(cursor_t* cursor, const char* text);

void cursor_set_max(cursor_t* cursor, u16 max_col, u16 max_row);

void cursor_down(cursor_t* cursor);

void cursor_up(cursor_t* cursor);

void cursor_right(cursor_t* cursor);

void cursor_left(cursor_t* cursor);

void cursor_break_line(cursor_t* cursor);

void cursor_delete_forward(cursor_t* cursor);

void cursor_delete_row(cursor_t* cursor);

void cursor_undo(cursor_t* cursor);
