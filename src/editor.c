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
  lp->nl = NULL;
  lp->pl = NULL;

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

void editor_delete_lines(line_t* lp, i32 size) {
  if (lp->pl == NULL && lp->nl == NULL) {
    lp->text[0] = '\0';
    lp->size = 0;
    return;
  }

  line_t *lp1, *lp2 = lp;
  i32 i = 0;
  do {
    lp1 = lp2;
    lp2 = lp1->nl;
    line_free(lp1);
  } while(lp2 != NULL && i++ < size);
}

line_t *editor_rows_to_string(line_t *head, unsigned int size) {
  line_t* ab = lalloc(0);
  line_t* lp = head;
  while (lp != NULL) {
    ab = line_append(ab, lp->text);
    ab = line_append(ab, "\n");
    lp = lp->nl;
  }

  return ab;
}

line_t* editor_insert_row_at(editor_t *E, usize y) {
  line_t *pl = E->lines;
  line_t *nl = NULL;

  for (i32 i = 0; i <= y; i++) {
    if (pl->nl == NULL) break;
    pl = pl->nl;
    nl = pl->nl;
  }

  line_t *lp = lalloc(0);
  lp->pl = pl;
  lp->nl = nl;

  pl->nl = lp;
  if (nl != NULL) nl->pl = lp;

  return pl;
}

line_t* editor_insert_row_with_data_at(editor_t *E, usize y, char* strdata) {
  line_t* lp = editor_insert_row_at(E, y);
  line_append(lp, strdata);
  return lp;
}

// TODO: REFACTOR
char editor_char_at(editor_t *E, i32 x, i32 y) {
  if (y > E->row_size || x < 0 || x > editor_rowlen(E, y))
    return '\0';

  return E->lines[y].text[x];
}

void editor_move_line_up(editor_t *E, i32 y) {
  if (y == 0)
    return;
  usize crow_len = editor_rowlen(E, y);
  usize prow_len = editor_rowlen(E, y - 1);

  line_t* prow = &E->lines[y-1];
  line_t* crow = &E->lines[y];

  if (crow_len + prow_len >= prow->size) {
    prow->size += crow_len;
    prow->text = nrealloc(prow->text, prow->size);
  }

  strcat(prow->text, crow->text);

  editor_delete_rows(E, y, y);
}


void editor_delete_char_at(editor_t* E, vec2_t pos) {
  usize len = editor_rowlen(E, pos.y);
  memmove(E->lines[pos.y].text + pos.x - 1,
          E->lines[pos.y].text + pos.x,
          len - pos.x + 1);
}

void editor_delete_between(editor_t* E, i32 y, i32 xs, i32 xe) {
  usize olen = editor_rowlen(E, y);
  if (xs == 0 && xe >= olen - 1) {
    E->lines[y].text[0] = '\0';
    return;
  }

  usize dsize = olen - xe;
  memmove(E->lines[y].text + xs, E->lines[y].text + xe, dsize);
  E->lines[y].text[xs + dsize] = '\0';
}

void editor_insert_char_at(editor_t *E, i32 x, i32 y, char ch) {
  line_t* row = &E->lines[y];
  u32 len = editor_rowlen(E, y);
  if (x < 0 || x > len)
    die("Invalid position x=%d", x);

  if (len + 1 >= row->size) {
    row->size += 8;
    row->text = nrealloc(row->text, row->size);
  }

  usize tmpsize = len - x;
  char *tmp = malloc(tmpsize);
  if (!tmp) return;

  memcpy(tmp, row->text + x, tmpsize);
  memcpy(row->text + x + 1, tmp, tmpsize);
  row->text[x] = ch;
  row->text[len + 1] = '\0';

  free(tmp);
}

void editor_break_line(editor_t *E, i32 x, i32 y) {
  editor_insert_row_at(E, y + 1);

  line_t* crow = &E->lines[y];
  line_t* nrow = &E->lines[y + 1];

  nrow->size = editor_rowlen(E, y) - x + 1;
  nrow->text = reallocstrcpy(nrow->text, crow->text + x, nrow->size);

  crow->text[x] = '\0';
  E->dirty = true;
}

void editor_delete_forward(editor_t *E, i32 x, i32 y) {
  E->lines[y].text[x] = '\0';
  E->dirty = true;
}

void editor_delete_backward(editor_t *E, i32 x, i32 y) {
  usize len = editor_rowlen(E, y) - x;
  memmove(E->lines[y].text, E->lines[y].text + x, len);
  E->lines[y].text[len] = '\0';
  E->dirty = true;
}

line_t *editor_text_between(editor_t *E, vec2_t start, vec2_t end) {
  line_t* lp = lalloc(0);

  for (i32 i = start.y; i <= end.y; i++) {
    i32 xs = i == start.y ? start.x : 0;
    i32 xe = i == end.y ? end.x : editor_rowlen(E, i);
    lp = line_append_s(lp, E->lines[i].text + xs, xe - xs);
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
    lp = line_append_s(lp, E->lines[i].text + xs, xe - xs);
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

void editor_insert_text(line_t* lp, i32 x, const char* strdata, usize dlen) {
  if (strdata == NULL || dlen == 0 || lp == NULL) return;

  char* aux = strdup(lp->text + lp->size);
  lp->text[lp->size] = '\0';

  line_append(lp, strdata);
  line_append(lp, aux);

  free(aux);
}

static void editor_new_file(const char *filename, editor_t *E) {
  strcpy(E->filename, filename);
  E->row_size = 1;
  E->lines = lalloc(BLOCK_SIZE);
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

  line_t* lp_tail = NULL;
  while (fgets(buffer, sizeof(buffer), fp)) {
    rows_size++;
    usize len = strlen(buffer);

    if (len > 0 && buffer[len - 1] == '\n') {
      buffer[len - 1] = '\0';
    }
    if (len > 1 && buffer[len - 2] == '\r') {
      buffer[len - 2] = '\0';
    }

    // build line
    line_t* line = lalloc(strlen(buffer));
    line = line_append(line, buffer);

    if (lp_tail == NULL) {
      E->lines = line;
      lp_tail = line;
      continue;
    }

    lp_tail->nl = line;
    line->pl = lp_tail;
    lp_tail = line;
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
