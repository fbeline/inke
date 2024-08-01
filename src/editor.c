#include "editor.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "utils.h"

char next(File* file, usize i) {
  if (i + 1 >= file->len)
    return '\0';

  return file->data[i + 1];
}

void editor_eol(editor_t* E) {
  i64 len = strlen(E->rows[E->cy].chars);
  E->cx = MIN(len, MAX_COL);
  E->coloff = MAX(0, len - MAX_COL);
}

void editor_bol(editor_t* E) {
  E->cx = 0;
  E->coloff = 0;
}

char* editor_rows_to_string(row_t* rows, unsigned int size) {
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

void editor_rowcpy(File* file, row_t* row, usize offset, usize eol) {
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

void editor_build_rows(editor_t *E, File *file) {
  usize rowSize = 10;
  E->rows = (row_t*)malloc(rowSize * sizeof(row_t));

  unsigned int currentRow = 0;
  usize lineStart = 0;
  for (usize i = 0; i < file->len; i++) {

    if (file->data[i] == '\n' || file->data[i] == '\r') {
      usize eol = i - 1;
      if (file->data[i] == '\r' && next(file, i) == '\n') {
        i++;
        eol--;
      }
      editor_rowcpy(file, &E->rows[currentRow], lineStart, i);

      currentRow++;
      lineStart = i + 1;
      if (currentRow >= rowSize) {
        rowSize *= 2;
        row_t *tmp = realloc(E->rows, rowSize * sizeof(row_t));
        E->rows = tmp;
      }
    }
  }

  E->rowSize = rowSize;
  E->rowslen = currentRow;
}

bool editor_insert_row_at(editor_t* E, usize n) {
  usize newLen = E->rowslen + 1;
  if (newLen >= E->rowSize) {
    usize newSize = E->rowSize + 10;
    row_t* tmp = (row_t*)realloc(E->rows, newSize * sizeof(row_t));
    if (tmp == NULL) return false;

    E->rowSize = newSize;
  }

  memmove(E->rows + n + 1, E->rows + n, sizeof(row_t) * (E->rowslen - n));
  E->rowslen = newLen;

  E->rows[n] = (row_t){0, NULL};
  return true;
}

void editor_remove_char_at_cursor(editor_t *E) {
  if (E->cx == 0 && E->coloff == 0) {
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
    memmove(E->rows + E->cy,
            E->rows + (E->cy + 1),
            (E->rowslen - E->cy - 1) * sizeof(row_t));

    E->rowslen--;
    E->cy--;
    E->cx = prow_len;
    return;
  }

  usize len = strlen(E->rows[E->cy].chars);
  memmove(E->rows[E->cy].chars + E->cx + E->coloff - 1, 
          E->rows[E->cy].chars + E->cx + E->coloff,
          len - E->cx - E->coloff + 1);

  if (E->cx == 0 && E->coloff > 0)
    E->coloff--;
  else
    E->cx--;
}

void editor_insert_char_at(row_t* row, int c, int i) {
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

void editor_insert_char_at_cursor(editor_t* E, int c) {
  if (strlen(E->rows[E->cy].chars) + 1 >= E->rows[E->cy].size) {
    E->rows[E->cy].size += 8;
    char* tmp = realloc(E->rows[E->cy].chars, E->rows[E->cy].size);
    if (tmp == NULL) {
      printf("MEM ALLOC FAILED\n");
      return;
    }
    E->rows[E->cy].chars = tmp;
  }
  editor_insert_char_at(&E->rows[E->cy], c, E->cx + E->coloff);
  E->cx++;

  if (E->cx > MAX_COL) {
    E->coloff++;
    E->cx = MAX_COL;
  }
}

editor_t editor_init(File* file) {
  editor_t E = { 0 };
  memcpy(E.filename, file->name, strlen(file->name));

  editor_build_rows(&E, file);

  return E;
}
