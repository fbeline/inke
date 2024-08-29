#include "editor.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "utils.h"

typedef struct append_buffer {
  usize capacity;
  usize size;
  char *b;
} abuf_t;

abuf_t abuf_init(usize capacity) {
  char *b = malloc(capacity);
  if (b == NULL)
    return (abuf_t){0};

  return (abuf_t){.capacity = capacity, .size = 0, .b = b};
}

void abuf_append(abuf_t *ab, const char *s) {
  bool resize = false;
  const usize len = strlen(s);

  while (ab->capacity <= ab->size + len) {
    ab->capacity *= 2;
    resize = true;
  }

  if (resize)
    ab->b = nrealloc(ab->b, ab->capacity);

  memcpy(ab->b + ab->size, s, len);
  ab->size += len;
  ab->b[ab->size] = '\0';
}

void abuf_free(abuf_t *ab) { free(ab->b); }

void editor_delete_rows(editor_t *E, i32 start, i32 end) {
  if (!IN_RANGE(start, 0, E->row_size) || !IN_RANGE(end, 0, E->row_size))
    return;

  usize n = end - start + 1;
  for (usize i = start; i <= end; i++) {
    free(E->rows[i].chars);
  }
  memmove(E->rows + start, E->rows + end + 1,
          (E->row_size - n) * sizeof(row_t));

  E->row_size -= n;
  E->dirty = true;
}

usize editor_rowlen(editor_t *E, i32 y) {
  if (y > E->row_size)
    return 0;

  return E->rows[y].size > 0 ? strlen(E->rows[y].chars) : 0;
}

char *editor_rows_to_string(row_t *rows, unsigned int size) {
  abuf_t ab = abuf_init(256);
  for (int i = 0; i < size; i++) {
    abuf_append(&ab, rows[i].chars);
    abuf_append(&ab, "\n");
  }

  return ab.b;
}

bool editor_insert_row_at(editor_t *E, usize n) {
  usize newLen = E->row_size + 1;
  if (newLen >= E->row_capacity) {
    usize newSize = E->row_capacity + 10;
    E->rows = (row_t *)realloc(E->rows, newSize * sizeof(row_t));
    E->row_capacity = newSize;
  }

  memmove(E->rows + n + 1, E->rows + n, sizeof(row_t) * (E->row_size - n));
  E->row_size = newLen;
  E->rows[n].size = 1;
  E->rows[n].chars = malloc(1);
  E->rows[n].chars[0] = '\0';
  E->dirty = true;

  return true;
}

void editor_insert_row_with_data_at(editor_t *E, usize y, char* strdata) {
  editor_insert_row_at(E, y);
  usize len = strlen(strdata);

  E->rows[y].size = len + 1;
  E->rows[y].chars = nrealloc(E->rows[y].chars, E->rows[y].size);

  memcpy(E->rows[y].chars, strdata, len);
}

char editor_char_at(editor_t *E, i32 x, i32 y) {
  if (y > E->row_size || x > editor_rowlen(E, y))
    return '\0';

  return E->rows[y].chars[x];
}

void editor_move_line_up(editor_t *E, i32 y) {
  if (y == 0)
    return;
  usize crow_len = editor_rowlen(E, y);
  usize prow_len = editor_rowlen(E, y - 1);

  // realloc previous row if necessary
  if (crow_len + prow_len >= E->rows[y - 1].size) {
    E->rows[y - 1].chars =
        nrealloc(E->rows[y - 1].chars, E->rows[y - 1].size + crow_len + 1);
    E->rows[y - 1].size += crow_len;
  }

  // cpy current row to the end of previous row
  if (crow_len > 0) {
    memcpy(E->rows[y - 1].chars + prow_len, E->rows[y].chars, crow_len);
    E->rows[y - 1].chars[crow_len + prow_len] = '\0';
  }

  editor_delete_rows(E, y, y);
}


void editor_delete_char_at(editor_t* E, vec2_t pos) {
  usize len = editor_rowlen(E, pos.y);
  memmove(E->rows[pos.y].chars + pos.x - 1,
          E->rows[pos.y].chars + pos.x,
          len - pos.x + 1);
}

void editor_insert_char_at(editor_t *E, i32 x, i32 y, char ch) {
  row_t row = E->rows[y];
  u32 len = editor_rowlen(E, y);
  if (x < 0 || x > len) {
    printf("Invalid position\n");
    return;
  }

  if (len + 1 >= row.size) {
    E->rows[y].size += 8;
    E->rows[y].chars = nrealloc(E->rows[y].chars, E->rows[y].size);
  }

  char *tmp = malloc(len - x + 1);
  if (!tmp)
    return;

  memcpy(tmp, E->rows[y].chars + x, len - x);

  E->rows[y].chars[x] = ch;
  memcpy(E->rows[y].chars + x + 1, tmp, len - x);
  E->rows[y].chars[len + 1] = '\0';

  free(tmp);
}

void editor_break_line(editor_t *E, i32 x, i32 y) {
  if (!editor_insert_row_at(E, y + 1))
    return;

  E->rows[y + 1].size = editor_rowlen(E, y) - x + 2;
  E->rows[y + 1].chars = malloc(E->rows[y + 1].size);
  memcpy(E->rows[y + 1].chars, E->rows[y].chars + x,
         strlen(E->rows[y].chars) - x + 1);

  E->rows[y].chars[x] = '\0';
  E->dirty = true;
}

void editor_delete_forward(editor_t *E, i32 x, i32 y) {
  E->rows[y].chars[x] = '\0';
  E->dirty = true;
}

void editor_delete_backward(editor_t *E, i32 x, i32 y) {
  usize len = editor_rowlen(E, y) - x;
  memmove(E->rows[y].chars, E->rows[y].chars + x, len);
  E->rows[y].chars[len] = '\0';
  E->dirty = true;
}

char *editor_text_between(editor_t *E, vec2_t start, vec2_t end) {
  abuf_t ab = abuf_init(16);

  for (i32 i = start.y; i <= end.y; i++) {
    i32 xs = i == start.y ? start.x : 0;
    i32 xe = i == end.y ? end.x : editor_rowlen(E, i);
    abuf_append(&ab, E->rows[i].chars + xs);
    if (i + 1 <= end.y) {
      abuf_append(&ab, "\n");
    }
  }
  return ab.b;
}

char *editor_cut_between(editor_t *E, vec2_t start, vec2_t end) {
  abuf_t ab = abuf_init(16);

  for (i32 i = start.y; i <= end.y; i++) {
    i32 xs = i == start.y ? start.x : 0;
    i32 xe = i == end.y ? end.x : editor_rowlen(E, i);
    abuf_append(&ab, E->rows[i].chars + xs);
    if (i + 1 <= end.y) {
      abuf_append(&ab, "\n");
    }
  }

  editor_delete_forward(E, start.x, start.y);

  i32 ldiff = end.y - start.y;
  if (ldiff >= 2)
    editor_delete_rows(E, start.y + 1, end.y - 1);

  if (ldiff > 0) {
    editor_delete_backward(E, end.x, end.y - ldiff + 1);
    editor_move_line_up(E, end.y - ldiff + 1);
  }

  return ab.b;
}

void editor_insert_text(editor_t* E, vec2_t pos, char* strdata, usize dlen) {
  if (strdata == NULL || dlen == 0) return;

  row_t* row = &E->rows[pos.y];
  usize clen = row->size > 0 ? strlen(row->chars) : 0;
  usize nlen = clen + dlen + 1;
  if (row->size == 0) {
    row->size = nlen + 1;
    row->chars = malloc(row->size);
  } else if (nlen >= row->size && row->size > 0) {
    row->size = nlen * 2;
    row->chars = nrealloc(row->chars, row->size);
  }

  memmove(row->chars + pos.x + dlen, row->chars + pos.x + 1, clen - pos.x);
  memcpy(row->chars + pos.x, strdata, dlen);

  row->chars[nlen] = '\0';
}

int editor_open_file(const char *filename, editor_t *E) {
  FILE *fp;
  char buffer[1024];
  usize rows_capacity = 0, rows_size = 0;

  fp = fopen(filename, "r");
  if (fp == NULL) {
    perror("Error opening file");
    return 1;
  }

  while (fgets(buffer, sizeof(buffer), fp)) {
    usize len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
      buffer[len - 1] = '\0';
    }
    if (len > 1 && buffer[len - 2] == '\r') {
      buffer[len - 2] = '\0';
    }

    // realoc rows if necessary
    if (rows_size >= rows_capacity) {
      rows_capacity = (rows_capacity == 0) ? 1 : rows_capacity * 2;
      E->rows = nrealloc(E->rows, rows_capacity * sizeof(row_t));
    }

    // build line
    E->rows[rows_size].size = strlen(buffer);
    E->rows[rows_size].chars = malloc(E->rows[rows_size].size + 1);
    if (E->rows[rows_size].chars == NULL) {
      perror("Error allocating memory for chars");
      fclose(fp);
      return 1;
    }
    memcpy(E->rows[rows_size].chars, buffer,
           E->rows[rows_size].size + 1); // +1 to cpy \0

    rows_size++;
  }

  fclose(fp);

  E->new_file = false;
  E->row_size = rows_size;
  E->row_capacity = rows_capacity;
  memcpy(E->filename, filename, strlen(filename));

  return 0;
}

void editor_new_file(const char *filename, editor_t *E) {
  memcpy(E->filename, filename, strlen(filename));
  E->row_size = 1;
  E->row_capacity = 1;
  E->rows = (row_t *)malloc(sizeof(row_t));
  E->rows->size = 1;
  E->rows->chars = malloc(1);
  E->rows->chars[0] = '\0';
  E->new_file = true;
}

editor_t editor_init(const char *filename) {
  editor_t E = {.running = true};

  if (IO_FILE_EXISTS(filename)) {
    editor_open_file(filename, &E);
  } else {
    editor_new_file(filename, &E);
  }

  return E;
}
