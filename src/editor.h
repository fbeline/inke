#pragma once

#include "types.h"

#define VERSION "0.0.1"

typedef struct line {
  struct line *nl;
  struct line *pl;

  usize capacity;
  usize size;
  char text[1];
} line_t;

typedef struct editor_s {
  unsigned char mode;
  char filename[255];
  u32 row_size, row_capacity;
  bool dirty;
  bool new_file;
  bool running;
  line_t* lines;
} editor_t;

line_t* line_append_s(line_t *lp, const char *str, usize len);

line_t* line_append(line_t *lp, const char *str);

void line_free(line_t *lp);

void editor_move_line_up(editor_t* E, i32 y);

void editor_delete_forward(editor_t* E, i32 x, i32 y);

void editor_delete_lines(line_t* lp, i32 size);

void editor_break_line(editor_t* E, i32 x, i32 y);

void editor_delete_char_at(editor_t* E, vec2_t pos);

void editor_insert_char_at(editor_t* E, i32 x, i32 y, char ch);

line_t* editor_text_between(editor_t* E, vec2_t start, vec2_t end);

line_t* editor_cut_between(editor_t* E, vec2_t start, vec2_t end);

char editor_char_at(editor_t* E, i32 x, i32 y);

line_t* editor_insert_row_at(editor_t* E, usize n);

line_t* editor_insert_row_with_data_at(editor_t *E, usize y, char* strdata);

void editor_insert_text(line_t* lp, i32 x, const char* str, usize dlen);

line_t* editor_rows_to_string(line_t* lines, unsigned int size);

editor_t editor_init(const char* filename);

