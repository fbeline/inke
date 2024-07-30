#include "editor.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

char Next(File* file, usize i) {
  if (i + 1 >= file->len)
    return '\0';

  return file->data[i + 1];
}

char* RowsToString(Row* rows, unsigned int size) {
  usize strsize = 1;
  usize strl = 0;
  char* str = malloc(strsize);
  str[0] = '\0';

  for (int i = 0; i < size; i++) {
    strsize += rows[i].size + 1;
    char* tmp = realloc(str, strsize);
    if (tmp == NULL) return NULL;
    str = tmp;

    memcpy(str + strl, rows[i].chars, strlen(rows[i].chars));
    strl += strlen(rows[i].chars);
    str[strl] = '\n';
    strl++;
  }
  str[strl] = '\0';

  return str;
}

void CpyRow(File* file, Row* row, usize offset, usize eol) {
  row->size = eol - offset + 2;
  row->chars = (char*)malloc(row->size);

  if (file->data[offset] == ' ') offset++; // do not render empty space as row first char

  for (usize k = 0, i = offset; i <= eol && i < file->len; k++) {
    i = offset + k;
    char c = file->data[i];
    if (c == '\n' || c == '\r') c = '\0';
    if (c == '\t') c = ' ';
    row->chars[k] = c;
  }
  row->chars[row->size - 1] = '\0';
}

void BuildRows(Editor *E, File *file) {
  usize rowSize = 10;
  E->rows = (Row*)malloc(rowSize * sizeof(Row));

  unsigned int currentRow = 0;
  usize lineStart = 0;
  for (usize i = 0; i < file->len; i++) {

    if (file->data[i] == '\n' || file->data[i] == '\r') {
      usize eol = i - 1;
      if (file->data[i] == '\r' && Next(file, i) == '\n') {
        i++;
        eol--;
      }
      CpyRow(file, &E->rows[currentRow], lineStart, i);

      currentRow++;
      lineStart = i + 1;
      if (currentRow >= rowSize) {
        rowSize *= 2;
        Row *tmp = realloc(E->rows, rowSize * sizeof(Row));
        E->rows = tmp;
      }
    }
  }

  E->rowSize = rowSize;
  E->rowslen = currentRow;
}

bool InsertRowAt(Editor* E, usize n) {
  usize newLen = E->rowslen + 1;
  if (newLen >= E->rowSize) {
    usize newSize = E->rowSize + 10;
    Row* tmp = (Row*)realloc(E->rows, newSize * sizeof(Row));
    if (tmp == NULL) return false;

    E->rowSize = newSize;
  }

  memmove(E->rows + n + 1, E->rows + n, sizeof(Row) * (E->rowslen - n));
  E->rowslen = newLen;

  E->rows[n] = (Row){0, NULL};
  return true;
}

void RemoveCharAtCursor(Editor *E) {
  if (E->cx == 0) {
    if (E->cy == 0) return;
    usize crow_len = strlen(E->rows[E->cy].chars);
    usize prow_len = strlen(E->rows[E->cy-1].chars);

    // realloc previous row if necessary
    if (crow_len + prow_len >= E->rows[E->cy-1].size) {
      char* tmp = realloc(E->rows[E->cy-1].chars, E->rows[E->cy-1].size + crow_len);
      if (tmp == NULL) return;
      E->rows[E->cy-1].chars = tmp;
      E->rows[E->cy-1].size += crow_len;
    }

    // cpy current row to the end of previous row
    memcpy(E->rows[E->cy-1].chars + prow_len, E->rows[E->cy].chars, crow_len);
    E->rows[E->cy-1].chars[crow_len + prow_len] = '\0';

    free(E->rows[E->cy].chars);
    memmove(E->rows + E->cy, E->rows + (E->cy + 1), (E->rowslen - E->cy - 1) * sizeof(Row));

    E->rowslen--;
    E->cy--;
    E->cx = prow_len;
    return;
  }

  usize len = strlen(E->rows[E->cy].chars);
  for (usize i = E->cx-1; i < len-1; i++) {
    E->rows[E->cy].chars[i] = E->rows[E->cy].chars[i + 1];
  }
  E->rows[E->cy].chars[len-1] = '\0';

  E->cx = E->cx - 1;
}

void InsertCharAt(Row* row, int c, int i) {
  u32 len = strlen(row->chars);
  if (i < 0 || i > len) {
    printf("Invalid position\n");
    return;
  }
  
  char* tmp = malloc(len - i + 1);
  if (!tmp) return;

  memcpy(tmp, row->chars + i, len - i);

  row->chars[i] = c;
  memcpy(row->chars + i + 1, tmp, len - i);
  row->chars[len + 1] = '\0';

  free(tmp);
}

void InsertChar(Editor* E, int c) {
  if (strlen(E->rows[E->cy].chars) + 1 >= E->rows[E->cy].size) {
    E->rows[E->cy].size += 8;
    char* tmp = realloc(E->rows[E->cy].chars, E->rows[E->cy].size);
    if (tmp == NULL) {
      printf("MEM ALLOC FAILED\n");
      return;
    }
    E->rows[E->cy].chars = tmp;
  }
  InsertCharAt(&E->rows[E->cy], c, E->cx);
  E->cx++;
}

Editor editor_init(File* file) {
  Editor E = { 0 };
  memcpy(E.filename, file->name, strlen(file->name));

  BuildRows(&E, file);

  return E;
}
