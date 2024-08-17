#include "editor.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "fs.h"
#include "utils.h"

char editor_char_at_cursor(editor_t* E) {
  char* chars = E->rows[E->cy].chars;
  usize index = E->cx + E->coloff;
  if (index > strlen(chars)) return '\0';

  return chars[index];
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

void editor_move_cursor(editor_t* E, i32 x, i32 y) {
  if (x < 0 && y == 0) return;

  i32 len = (i32)strlen(E->rows[y].chars);

  if (x < 0 && E->coloff > 0) {
    E->coloff--;
  } else if (x < 0 && E->coloff == 0) {
    y = MAX(0, y - 1);
    x = (i32)strlen(E->rows[y].chars);
    E->coloff = 0;
    editor_move_cursor(E, x, y);
  } else if (x <= len && x >= MAX_COL) {
    E->coloff = x - MAX_COL;
    E->cx = x - E->coloff;
  } else if (x > len) {
    E->coloff = 0;
    E->cx = 0;
    y++;
  } else {
    E->cx = x;
  }

  E->cy = y;
}

void editor_move_cursor_word_forward(editor_t* E) {
  char ch;
  do {
    if (E->cy >= E->row_size - 1 &&
        strlen(E->rows[E->cy].chars) >= E->cx) break;
    editor_move_cursor(E, E->cx + E->coloff + 1, E->cy);
  } while((ch = editor_char_at_cursor(E)), ch  !=  ' ' && ch != '\0');
}

void editor_move_cursor_word_backward(editor_t* E) {
  char ch;
  do {
    if (E->cy == 0 && E->cx == 0) break;
    editor_move_cursor(E, E->cx + E->coloff - 1, E->cy);
  } while((ch = editor_char_at_cursor(E)), ch  !=  ' ' && ch != '\0');
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

bool editor_insert_row_at(editor_t* E, usize n) {
  usize newLen = E->row_size + 1;
  if (newLen >= E->row_capacity) {
    usize newSize = E->row_capacity + 10;
    row_t* tmp = (row_t*)realloc(E->rows, newSize * sizeof(row_t));
    if (tmp == NULL) return false;

    E->row_capacity = newSize;
  }

  memmove(E->rows + n + 1, E->rows + n, sizeof(row_t) * (E->row_size - n));
  E->row_size = newLen;
  E->rows[n] = (row_t){0, NULL};
  E->dirty = true;

  return true;
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
            (E->row_size - E->cy - 1) * sizeof(row_t));

    editor_move_cursor(E, prow_len, E->cy-1);
    E->row_size--;

    E->dirty = true;
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

  E->dirty = true;
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

  E->dirty = true;
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
  E->dirty = true;
}

void editor_delete_forward(editor_t* E) {
  i32 index = E->coloff + E->cx;
  row_t row = E->rows[E->cy];

  row.chars[index] = '\0';
  E->dirty = true;
}

int editor_open_file(const char* filename, editor_t* E) {
  FILE* fp;
  char buffer[1024];
  usize rows_capacity = 0, rows_size = 0;

  fp = fopen(filename, "r");
  if (fp == NULL) {
    perror("Error opening file");
    return 1;
  }

  while(fgets(buffer, sizeof(buffer), fp)) {
    usize len = strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n') {
      buffer[len-1] = '\0';
    }
    if (len > 1 && buffer[len-2] == '\r') {
      buffer[len-2] = '\0';
    }

    // realoc rows if necessary
    if (rows_size >= rows_capacity) {
      rows_capacity = (rows_capacity == 0) ? 1 : rows_capacity * 2;
      E->rows = realloc(E->rows, rows_capacity * sizeof(row_t));
      if (E->rows == NULL) {
        perror("Error reallocating memory");
        fclose(fp);
        return 1;
      }
    }

    // build line
    E->rows[rows_size].size = strlen(buffer);
    E->rows[rows_size].chars = malloc(E->rows[rows_size].size + 1);
    if (E->rows[rows_size].chars == NULL) {
      perror("Error allocating memory for chars");
      fclose(fp);
      return 1;
    }
    memcpy(E->rows[rows_size].chars, buffer, E->rows[rows_size].size + 1); // +1 to cpy \0

    rows_size++;
  }

  fclose(fp);

  E->new_file = false;
  E->row_size = rows_size;
  E->row_capacity = rows_capacity;
  memcpy(E->filename, filename, strlen(filename));

  return 0;
}

void editor_new_file(const char* filename, editor_t* E) {
  memcpy(E->filename, filename, strlen(filename));
  E->row_size = 1;
  E->row_capacity = 1;
  E->rows = (row_t*)malloc(sizeof(row_t));
  E->rows->size = 1;
  E->rows->chars = malloc(1);
  E->rows->chars[0] = '\0';
  E->new_file = true;
}

editor_t editor_init(const char* filename) {
  editor_t E = { .running = true };

  if (FS_FILE_EXISTS(filename)) {
    editor_open_file(filename, &E);
  } else {
    editor_new_file(filename, &E);
  }

  return E;
}
