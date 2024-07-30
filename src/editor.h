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


char* RowsToString(Row* rows, unsigned int size);

void BuildRows(File *file);

bool InsertRowAt(usize n);

void RemoveCharAtCursor(void);

void InsertCharAt(Row* row, int c, int i);

void InsertChar(int c);


extern Editor E;
