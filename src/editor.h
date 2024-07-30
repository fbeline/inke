#pragma once

#include <raylib.h>
#include "types.h"
#include "fs.h"

#define MAX_COL 66
#define DUMMY_LINE "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

typedef struct editorFont {
  Font font;
  unsigned short size;
  unsigned short lineSpacing;
} EditorFont;

typedef struct window {
  int width;
  int height;
} Window;

typedef struct row {
  usize size;
  char* chars;
} Row;

typedef struct editor {
  char filename[255];
  int cx, cy;
  int rowoff;
  int coloff;
  int screenrows;
  int screencols;
  Vector2 eMargin;
  EditorFont font;
  unsigned int rowslen;
  unsigned int rowSize;
  Row* rows;
  Window window;
} Editor;

Editor editor_init(File *file);

void editor_remove_char_at_cursor(Editor *E);

void editor_insert_char_at_cursor(Editor* E, int c);

char* editor_rows_to_string(Row* rows, unsigned int size);

bool editor_insert_row_at(Editor* E, usize n);
