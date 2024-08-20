#include "editor.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "fs.h"
#include "utils.h"

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

char editor_char_at(editor_t* E, i32 x, i32 y) {
  if (y > E->row_size || x > strlen(E->rows[y].chars))
    return '\0';

  return E->rows[y].chars[x];
}

void editor_move_line_up(editor_t* E, i32 y) {
    if (y == 0) return;
    usize crow_len = strlen(E->rows[y].chars);
    usize prow_len = strlen(E->rows[y-1].chars);

    // realloc previous row if necessary
    if (crow_len + prow_len >= E->rows[y-1].size) {
      char* tmp = realloc(E->rows[y-1].chars, E->rows[y-1].size + crow_len + 1);
      if (tmp == NULL) return;
      E->rows[y-1].chars = tmp;
      E->rows[y-1].size += crow_len;
    }

    // cpy current row to the end of previous row
    if (crow_len > 0) {
      memcpy(E->rows[y-1].chars + prow_len, E->rows[y].chars, crow_len);
      E->rows[y-1].chars[crow_len + prow_len] = '\0';
    }

    // remove row
    free(E->rows[y].chars);
    memmove(E->rows + y,
            E->rows + (y + 1),
            (E->row_size - y - 1) * sizeof(row_t));

    E->row_size--;
    E->dirty = true;
}

void editor_insert_char_at(editor_t* E, i32 x, i32 y, char ch) {
  row_t row = E->rows[y];
  u32 len = strlen(row.chars);
  if (x < 0 || x > len) {
    printf("Invalid position\n");
    return;
  }

  if (len + 1 >= row.size) {
    E->rows[y].size += 8;
    char* tmp = realloc(E->rows[y].chars, E->rows[y].size);
    if (tmp == NULL) return;
    E->rows[y].chars = tmp;
  }
  
  char* tmp = malloc(len - x + 1);
  if (!tmp) return;

  memcpy(tmp, E->rows[y].chars + x, len - x);

  E->rows[y].chars[x] = ch;
  memcpy(E->rows[y].chars + x + 1, tmp, len - x);
  E->rows[y].chars[len + 1] = '\0';

  free(tmp);
}

void editor_break_line(editor_t* E, i32 x, i32 y) {
  if (!editor_insert_row_at(E, y + 1)) return;

  E->rows[y + 1].size = strlen(E->rows[y].chars) - x+ 2;
  E->rows[y + 1].chars = malloc(E->rows[y + 1].size);
  memcpy(E->rows[y + 1].chars,
         E->rows[y].chars + x,
         strlen(E->rows[y].chars) - x + 1);

  E->rows[y].chars[x] = '\0';
  E->dirty = true;
}

void editor_delete_forward(editor_t* E, i32 x, i32 y) {
  E->rows[y].chars[x] = '\0';
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
