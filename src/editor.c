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

void editor_move_cursor_right(editor_t *E) {
    E->cx++;
    if (E->cx + E->coloff > MAX_COL) {
      E->cx = MAX_COL;
      E->coloff++;
    }
    if (E->cx + E->coloff > strlen(E->rows[E->cy].chars)) {
      E->coloff = 0;
      E->cx = 0;
      E->cy++;
    }
}

void editor_move_cursor_left(editor_t *E) {
  E->cx--;

  if (E->cx < 0) {
    if (E->cy == 0) {
      E->cx = 0;
      return;
    }
    int rowlen = strlen(E->rows[MAX(0, E->cy-1)].chars);
    if (E->coloff == 0) {
      E->cx = MIN(rowlen, MAX_COL);
      E->cy = MAX(E->cy - 1, 0);
      if (rowlen > MAX_COL) E->coloff = rowlen - MAX_COL;
    } else {
      E->coloff--;
      E->cx = 0;
    }
  }
}

void editor_move_cursor_word_forward(editor_t* E) {
  do {
    if (E->cy >= E->rowslen - 1 && strlen(E->rows[E->cy].chars) >= E->cx) break;
    editor_move_cursor_right(E);
  } while(E->rows[E->cy].chars[E->cx] !=  ' ');
}

void editor_move_cursor_word_backward(editor_t* E) {
  do {
    if (E->cy == 0 && E->cx == 0) break;
    editor_move_cursor_left(E);
  } while(E->rows[E->cy].chars[E->cx] !=  ' ');
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

void editor_move_cursor(editor_t* E, i32 x, i32 y) {
  usize len = strlen(E->rows[y].chars);
  if (x <= len && x > MAX_COL) {
    const i32 offset = 10;
    E->coloff = x - MAX_COL + offset;
    E->cx = MAX_COL - offset;
  } else if (x > len) {
    E->cx = len;
  } else {
    E->cx = x;
  }

  E->cy = y;
}

void editor_move_line_up(editor_t* E) {
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
    if (crow_len > 0) {
      memcpy(E->rows[E->cy-1].chars + prow_len, E->rows[E->cy].chars, crow_len);
      E->rows[E->cy-1].chars[crow_len + prow_len] = '\0';
    }

    // remove row
    free(E->rows[E->cy].chars);
    memmove(E->rows + E->cy,
            E->rows + (E->cy + 1),
            (E->rowslen - E->cy - 1) * sizeof(row_t));

    editor_move_cursor(E, prow_len, E->cy-1);
    E->rowslen--;
}

void editor_remove_char_at_cursor(editor_t *E) {
  if (E->cx == 0 && E->coloff == 0) {
    return editor_move_line_up(E);
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
    if (tmp == NULL) return;
    E->rows[E->cy].chars = tmp;
  }
  editor_insert_char_at(&E->rows[E->cy], c, E->cx + E->coloff);

  E->cx++;
  if (E->cx > MAX_COL) {
    E->coloff++;
    E->cx = MAX_COL;
  }
}

void editor_return(editor_t* E) {
  if (!editor_insert_row_at(E, E->cy + 1)) return;

  E->rows[E->cy + 1].size = strlen(E->rows[E->cy].chars) - E->cx - E->coloff + 2;
  E->rows[E->cy + 1].chars = malloc(E->rows[E->cy + 1].size);
  memcpy(E->rows[E->cy + 1].chars,
         E->rows[E->cy].chars + E->cx + E->coloff,
         strlen(E->rows[E->cy].chars) - E->cx - E->coloff + 1);

  E->rows[E->cy].chars[E->cx + E->coloff] = '\0';

  editor_move_cursor(E, 0, E->cy+1);
  E->coloff = 0;
}

editor_t editor_init(File* file) {
  editor_t E = { 0 };
  memcpy(E.filename, file->name, strlen(file->name));

  editor_build_rows(&E, file);

  return E;
}
