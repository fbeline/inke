#pragma once

#include "types.h"
#include "fs.h"

#define MAX_COL 66
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
  u32 rowslen, rowSize;
  row_t* rows;
} editor_t;

editor_t editor_init(File *file);

void editor_bol(editor_t* E);

void editor_eol(editor_t* E);

void editor_move_cursor_right(editor_t *E);

void editor_move_cursor_left(editor_t *E);

void editor_remove_char_at_cursor(editor_t* E);

void editor_insert_char_at_cursor(editor_t* E, int c);

char* editor_rows_to_string(row_t* rows, unsigned int size);

bool editor_insert_row_at(editor_t* E, usize n);

void editor_return(editor_t* E);
