#include "editor.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "utils.h"

#define BLOCK_SIZE 16

line_t *lalloc(usize size) {
  line_t *lp;
  usize capacity;

  capacity = (size + BLOCK_SIZE - 1) & ~(BLOCK_SIZE - 1);
  if (capacity == 0)
    capacity = BLOCK_SIZE;

  if ((lp = (line_t*)malloc(sizeof(line_t) + capacity)) == NULL)
    die("LALLOC OUT OF MEMORY");

  lp->capacity = capacity;
  lp->size = size;
  lp->text[0] = '\0';

  return lp;
}

line_t *lrealloc(line_t *lp, usize new_size) {
  line_t *nlp;
  usize new_capacity;

  new_capacity = (new_size + BLOCK_SIZE - 1) & ~(BLOCK_SIZE - 1);
  if (new_capacity == 0)
    new_capacity = BLOCK_SIZE;

  if ((nlp = (line_t*)realloc(lp, sizeof(line_t) + new_capacity)) == NULL)
    die("LREALLOC OUT OF MEMORY");

  nlp->capacity = new_capacity;
  nlp->size = new_size;

  return nlp;
}

line_t* line_append_s(line_t *lp, const char *str, usize len) {
  usize old_size = lp->size;
  usize new_size = lp->size + len;
  if (new_size > lp->capacity)
    lp = lrealloc(lp, new_size);

  for(int i = 0, j = old_size; i < len; i++, j++) {
    lp->text[j] = str[i];
  }

  lp->text[new_size] = '\0';
  lp->size = new_size;

  return lp;
}

line_t* line_append(line_t *lp, const char *str) {
  return line_append_s(lp, str, strlen(str));
}

void line_free(line_t *lp) { 
  free((char*) lp); 
}

void editor_delete_rows(editor_t *E, i32 start, i32 end) {
  E->dirty = true;
  if (E->row_size == 1) {
    E->rows[0].chars[0] = '\0';
    return;
  }

  start = MAX(0, start);
  start = MIN(start, (i32)E->row_size-1);
  end = MIN(end, (i32)E->row_size-1);
  end = MAX(end, 0);

  if (end < start)
    die("editor_delete_rows invalid args start=%d, end=%d", start, end);

  for(i32 i = start; i <= end; i++) {
    free(E->rows[i].chars);
  }

  // all lines deleted
  if (start == 0 && end == E->row_size - 1) {
    E->rows[0].chars = malloc(1);
    E->rows[0].chars[0] = '\0';
    E->row_size = 1;
    return;
  }

  i32 n = end - start + 1;
  if (end < E->row_size - 1) // do not move if end == last line
    memmove(E->rows + start, E->rows + end + 1, (E->row_size - end - 1) * sizeof(row_t));

  E->row_size -= n;
}

usize editor_rowlen(editor_t *E, i32 y) {
  if (y >= E->row_size)
    return 0;

  return E->rows[y].size > 0 ? strlen(E->rows[y].chars) : 0;
}

line_t *editor_rows_to_string(row_t *rows, unsigned int size) {
  line_t* ab = lalloc(0);
  for (int i = 0; i < size; i++) {
    ab = line_append(ab, rows[i].chars);
    ab = line_append(ab, "\n");
  }

  return ab;
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

line_t *editor_text_between(editor_t *E, vec2_t start, vec2_t end) {
  line_t* lp = lalloc(0);

  for (i32 i = start.y; i <= end.y; i++) {
    i32 xs = i == start.y ? start.x : 0;
    i32 xe = i == end.y ? end.x : editor_rowlen(E, i);
    lp = line_append_s(lp, E->rows[i].chars + xs, xe - xs);
    if (i + 1 <= end.y) {
      lp = line_append(lp, "\n");
    }
  }
  return lp;
}

line_t *editor_cut_between(editor_t *E, vec2_t start, vec2_t end) {
  line_t *lp = lalloc(0);

  for (i32 i = start.y; i <= end.y; i++) {
    i32 xs = i == start.y ? start.x : 0;
    i32 xe = i == end.y ? end.x : editor_rowlen(E, i);
    lp = line_append_s(lp, E->rows[i].chars + xs, xe - xs);
    if (i + 1 <= end.y) {
      lp = line_append(lp, "\n");
    }
    editor_delete_between(E, i, xs, xe);
  }

  i32 ldiff = end.y - start.y;
  if (ldiff >= 2)
    editor_delete_rows(E, start.y + 1, end.y - 1);

  if (ldiff > 0)
    editor_move_line_up(E, end.y - ldiff + 1);

  return lp;
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

  if (pos.x == 0 || pos.x >= clen) {
    memmove(row->chars + pos.x + dlen, row->chars + pos.x, clen - pos.x);
    memcpy(row->chars + pos.x, strdata, dlen);
    row->chars[nlen-1] = '\0';
  } else {
    memmove(row->chars + pos.x + dlen, row->chars + pos.x, clen - pos.x + 1);
    memcpy(row->chars + pos.x, strdata, dlen);
    row->chars[nlen] = '\0';
  }
}

static void editor_new_file(const char *filename, editor_t *E) {
  strcpy(E->filename, filename);
  E->row_size = 1;
  E->row_capacity = 1;
  E->rows = (row_t *)malloc(sizeof(row_t));
  E->rows->size = 1;
  E->rows->chars = malloc(1);
  E->rows->chars[0] = '\0';
  E->new_file = true;
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

  E->row_size = rows_size;
  E->row_capacity = rows_capacity;
  strcpy(E->filename, filename);

  if (rows_size == 0) editor_new_file(filename, E);
  E->new_file = false;

  return 0;
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
