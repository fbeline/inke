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

void abuf_append_s(abuf_t *ab, const char *s, usize len) {
  bool resize = false;

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

void abuf_append(abuf_t *ab, const char *s) {
  abuf_append_s(ab, s, strlen(s));
}

void abuf_free(abuf_t *ab) { free(ab->b); }

void editor_delete_rows(editor_t *E, i32 start, i32 end) {
  if (!IN_RANGE(start, 0, E->row_size) || !IN_RANGE(end, 0, E->row_size))
    return;

  usize n = end - start + 1;
  for (usize i = start; i <= end; i++) {
    free(E->rows[i].chars);
  }

  if (E->row_size - end - 1 > 0)
    memmove(E->rows + start, E->rows + end + 1, (E->row_size - n) * sizeof(row_t));

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

void editor_insert_row_at(editor_t *E, usize n) {
  if (++E->row_size >= E->row_capacity) {
    E->row_capacity += 10;
    E->rows = nrealloc(E->rows, E->row_capacity * sizeof(row_t));
  }

  memmove(E->rows + n + 1, E->rows + n, sizeof(row_t) * (E->row_size - n));
  E->rows[n].size = 1;
  E->rows[n].chars = malloc(1);
  E->rows[n].chars[0] = '\0';
  E->dirty = true;
}

void editor_insert_row_with_data_at(editor_t *E, usize y, char* strdata) {
  editor_insert_row_at(E, y);

  row_t* row = &E->rows[y];
  row->size = strlen(strdata) + 1;
  row->chars = reallocstrcpy(row->chars, strdata, row->size);
}

char editor_char_at(editor_t *E, i32 x, i32 y) {
  if (y > E->row_size || x < 0 || x > editor_rowlen(E, y))
    return '\0';

  return E->rows[y].chars[x];
}

void editor_move_line_up(editor_t *E, i32 y) {
  if (y == 0)
    return;
  usize crow_len = editor_rowlen(E, y);
  usize prow_len = editor_rowlen(E, y - 1);

  row_t* prow = &E->rows[y-1];
  row_t* crow = &E->rows[y];

  if (crow_len + prow_len >= prow->size) {
    prow->size += crow_len;
    prow->chars = nrealloc(prow->chars, prow->size);
  }

  strcat(prow->chars, crow->chars);

  editor_delete_rows(E, y, y);
}


void editor_delete_char_at(editor_t* E, vec2_t pos) {
  usize len = editor_rowlen(E, pos.y);
  memmove(E->rows[pos.y].chars + pos.x - 1,
          E->rows[pos.y].chars + pos.x,
          len - pos.x + 1);
}

void editor_delete_between(editor_t* E, i32 y, i32 xs, i32 xe) {
  usize olen = editor_rowlen(E, y);
  if (xs == 0 && xe >= olen - 1) {
    E->rows[y].chars[0] = '\0';
    return;
  }

  usize dsize = olen - xe;
  memmove(E->rows[y].chars + xs, E->rows[y].chars + xe, dsize);
  E->rows[y].chars[xs + dsize] = '\0';
}

void editor_insert_char_at(editor_t *E, i32 x, i32 y, char ch) {
  row_t* row = &E->rows[y];
  u32 len = editor_rowlen(E, y);
  if (x < 0 || x > len)
    die("Invalid position x=%d", x);

  if (len + 1 >= row->size) {
    row->size += 8;
    row->chars = nrealloc(row->chars, row->size);
  }

  usize tmpsize = len - x;
  char *tmp = malloc(tmpsize);
  if (!tmp) return;

  memcpy(tmp, row->chars + x, tmpsize);
  memcpy(row->chars + x + 1, tmp, tmpsize);
  row->chars[x] = ch;
  row->chars[len + 1] = '\0';

  free(tmp);
}

void editor_break_line(editor_t *E, i32 x, i32 y) {
  editor_insert_row_at(E, y + 1);

  row_t* crow = &E->rows[y];
  row_t* nrow = &E->rows[y + 1];

  nrow->size = editor_rowlen(E, y) - x + 1;
  nrow->chars = reallocstrcpy(nrow->chars, crow->chars + x, nrow->size);

  crow->chars[x] = '\0';
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
    abuf_append_s(&ab, E->rows[i].chars + xs, xe - xs);
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
    abuf_append_s(&ab, E->rows[i].chars + xs, xe - xs);
    if (i + 1 <= end.y) {
      abuf_append(&ab, "\n");
    }
    editor_delete_between(E, i, xs, xe);
  }

  i32 ldiff = end.y - start.y;
  if (ldiff >= 2)
    editor_delete_rows(E, start.y + 1, end.y - 1);

  if (ldiff > 0)
    editor_move_line_up(E, end.y - ldiff + 1);

  return ab.b;
}

void editor_insert_text(editor_t* E, vec2_t pos, const char* strdata, usize dlen) {
  if (strdata == NULL || dlen == 0) return;

  row_t* row = &E->rows[pos.y];
  usize clen = row->size > 0 ? strlen(row->chars) : 0;
  usize nlen = clen + dlen + 1;
  if (nlen >= row->size && row->size > 0) {
    row->size = nlen * 2;
    row->chars = nrealloc(row->chars, row->size);
  }

  memmove(row->chars + pos.x + dlen, row->chars + pos.x, clen - pos.x);
  memcpy(row->chars + pos.x, strdata, dlen);

  row->chars[nlen-1] = '\0';
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
    row_t* row = &E->rows[rows_size];
    row->size = strlen(buffer) + 1;
    row->chars = malloc(row->size);
    if (row->chars == NULL) die("out of memory");

    memcpy(row->chars, buffer, row->size);

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
