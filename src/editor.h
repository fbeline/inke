#pragma once

#include "types.h"

#define VERSION "0.0.1"

#define MAX_COL 66
#define MAX_ROW 19
#define DUMMY_LINE "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

typedef struct row_s {
  usize size;
  char* chars;
} row_t;

typedef struct editor_s {
  char filename[255];
  i32 cx, cy;
  i32 rowoff, coloff;
  i32 screenrows, screencols;
  u32 row_size, row_capacity;
  bool dirty;
  bool new_file;
  bool running;
  row_t* rows;
} editor_t;

void editor_bol(editor_t* E);

void editor_eol(editor_t* E);

void editor_move_cursor(editor_t* E, i32 x, i32 y);

void editor_move_cursor_word_forward(editor_t* E);

void editor_move_cursor_word_backward(editor_t* E);

void editor_remove_char_at_cursor(editor_t* E);

void editor_insert_char_at_cursor(editor_t* E, int c);

void editor_delete_forward(editor_t* E);

char* editor_rows_to_string(row_t* rows, unsigned int size);

void editor_return(editor_t* E);

editor_t editor_init(const char* filename);
