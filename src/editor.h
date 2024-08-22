#pragma once

#include "types.h"

#define VERSION "0.0.1"

typedef struct row_s {
  usize size;
  char* chars;
} row_t;

typedef struct editor_s {
  char filename[255];
  u32 row_size, row_capacity;
  bool dirty;
  bool new_file;
  bool running;
  row_t* rows;
} editor_t;

void editor_move_line_up(editor_t* E, i32 y);

void editor_delete_forward(editor_t* E, i32 x, i32 y);

void editor_delete_row(editor_t* E, i32 y);

void editor_break_line(editor_t* E, i32 x, i32 y);

void editor_insert_char_at(editor_t* E, i32 x, i32 y, char ch);

char* editor_text_between(editor_t* E, vec2_t start, vec2_t end);

char* editor_cut_between(editor_t* E, vec2_t start, vec2_t end);

char editor_char_at(editor_t* E, i32 x, i32 y);

usize editor_rowlen(editor_t* E, i32 y);

bool editor_insert_row_at(editor_t* E, usize n);

char* editor_rows_to_string(row_t* rows, unsigned int size);

editor_t editor_init(const char* filename);

