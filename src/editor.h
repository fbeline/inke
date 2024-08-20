#pragma once

#include "types.h"

#define VERSION "0.0.1"

#define MAX_ROW 24
#define DUMMY_LINE "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

typedef struct row_s {
  usize size;
  char* chars;
} row_t;

typedef struct editor_s {
  char filename[255];
  i32 screenrows, screencols;
  u32 row_size, row_capacity;
  bool dirty;
  bool new_file;
  bool running;
  row_t* rows;
} editor_t;

void editor_move_line_up(editor_t* E, i32 cy);

void editor_delete_forward(editor_t* E, i32 cx, i32 cy);

void editor_break_line(editor_t* E, i32 cx, i32 cy);

void editor_insert_char_at(row_t* row, int c, int i);

char editor_char_at(editor_t* E, i32 x, i32 y);

bool editor_insert_row_at(editor_t* E, usize n);

char* editor_rows_to_string(row_t* rows, unsigned int size);

void editor_return(editor_t* E);

editor_t editor_init(const char* filename);

