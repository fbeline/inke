#include "editor.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "utils.h"

#define BLOCK_SIZE 16

line_t *lalloc(usize capacity) {
  line_t *lp;

  if ((lp = (line_t*)malloc(sizeof(line_t))) == NULL)
    DIE("LALLOC OUT OF MEMORY");

  lp->ds = dsnewlen(capacity);

  lp->next = NULL;
  lp->prev = NULL;

  return lp;
}

void line_free(line_t *lp) {
  if (lp == NULL) return;

  if (lp->prev != NULL) lp->prev->next = lp->next;
  if (lp->next != NULL) lp->next->prev = lp->prev;

  dsfree(lp->ds);
  free(lp);
}

static line_t *editor_create_line_after(editor_t *E, line_t* lp, usize capacity) {
  line_t *nlp = lalloc(capacity);

  nlp->prev = lp;
  nlp->next = lp->next;

  if (nlp->next) nlp->next->prev = nlp;
  if (nlp->prev) nlp->prev->next = nlp;

  E->row_size++;

  return nlp;
}

void editor_delete_lines(editor_t *E, line_t* lp, usize size) {
  line_t *lp1, *lp2 = lp;
  usize i = 0;
  do {
    lp1 = lp2;
    lp2 = lp1->next;

    // buffer must have at least 1 line
    if (lp1->prev == NULL && lp1->next == NULL) {
      lp1->ds->buf[0] = '\0';
      lp1->ds->len = 0;
      E->lines = lp1;
      E->row_size = 1;
      return;
    }

    line_free(lp1);
  } while(lp2 != NULL && ++i < size);

  if (lp == E->lines)
    E->lines = lp2;

  E->row_size -= size;
}

ds_t *editor_rows_to_string(line_t *head) {
  ds_t *ds = dsempty();
  line_t* lp = head;
  while (lp != NULL) {
    dsrtrim(lp->ds);
    dscat(ds, lp->ds->buf);
    dscat(ds, "\n");
    lp = lp->next;
  }

  return ds;
}

line_t* editor_insert_row_at(editor_t *E, u32 y) {
  line_t *pl = NULL;
  line_t *nl = E->lines;

  usize i = 0;
  while(++i <= y && nl != NULL) {
    pl = nl;
    nl = nl->next;
  }

  line_t *lp = lalloc(0);
  lp->prev = pl;
  lp->next = nl;

  if (pl != NULL) pl->next = lp;
  if (nl != NULL) nl->prev = lp;
  if (y == 0) E->lines = lp;

  E->row_size++;
  E->dirty = true;

  return lp;
}

line_t* editor_insert_row_with_data_at(editor_t *E, u32 y, char* strdata) {
  line_t* lp = editor_insert_row_at(E, y);
  dscat(lp->ds, strdata);

  return lp;
}

char editor_char_at(line_t *lp, u32 x) {
  if (lp == NULL || x > lp->ds->len)
    return '\0';

  return lp->ds->buf[x];
}

line_t *editor_move_line_up(editor_t *E, line_t *lp) {
  if (lp == NULL || lp->prev == NULL) return NULL;

  dscat(lp->prev->ds, lp->ds->buf);

  E->dirty = true;
  E->row_size--;
  line_t *prev = lp->prev;
  line_free(lp);

  return prev;
}

void editor_delete_char_at(line_t *lp, u32 x) {
  if (x >= lp->ds->len || lp == NULL) return;

  memmove(&lp->ds->buf[x],
          &lp->ds->buf[x+1],
          lp->ds->len - x + 1);

  lp->ds->len--;
}

line_t *editor_insert_char_at(editor_t *E, line_t *lp, u32 x, char ch) {
  if (x > lp->ds->len)
    DIE("Invalid position x=%d", x);

  dsichar(lp->ds, x, ch);

  return lp;
}

void editor_break_line(editor_t *E, line_t *lp, u32 x) {
  if (lp == NULL || x > lp->ds->len) return;

  line_t *new_line = lalloc(16);
  line_t *next_line = lp->next;

  dscat(new_line->ds, lp->ds->buf + x);
  lp->ds->buf[x] = '\0';
  lp->ds->len = x;

  if (next_line != NULL) next_line->prev = new_line;
  lp->next = new_line;
  new_line->prev = lp;
  new_line->next = next_line;

  E->dirty = true;
  E->row_size++;
}

void editor_delete_forward(line_t *lp, u32 x) {
  lp->ds->buf[x] = '\0';
  lp->ds->len = x;
}

void editor_delete_backward(line_t *lp, u32 x) {
  lp->ds->len = lp->ds->len > x ? lp->ds->len - x : 0;
  memmove(lp->ds->buf, lp->ds->buf + x, lp->ds->len);
  lp->ds->buf[lp->ds->len] = '\0';
}

void editor_text_between(editor_t *E, mark_t mark, ds_t *r) {
  r->buf[0] = '\0';
  r->len = 0;

  if (mark.start_lp == mark.end_lp) {
    dsncat(r,
           mark.start_lp->ds->buf + mark.start_offset,
           mark.end_offset - mark.start_offset);
    return;
  }

  line_t *lp = mark.start_lp;
  dscat(r, lp->ds->buf + mark.start_offset);
  dscat(r, "\n");
  lp = lp->next;

  while(lp != mark.end_lp && lp != NULL) {
    dscat(r, lp->ds->buf);
    dscat(r, "\n");
    lp = lp->next;
  }

  dsncat(r, lp->ds->buf, mark.end_offset);
}

void editor_kill_between(editor_t *E, mark_t mark, ds_t *r) {
  line_t *lp = mark.start_lp;

  editor_text_between(E, mark, r);

  if (lp == mark.end_lp) {
    usize mark_size = mark.end_offset - mark.start_offset;

    memmove(lp->ds->buf + mark.start_offset,
            lp->ds->buf + mark.end_offset,
            lp->ds->len - mark.end_offset);
    lp->ds->len = mark_size <= lp->ds->len ? lp->ds->len - mark_size : 0;
    lp->ds->buf[lp->ds->len] = '\0';
    return;
  }

  lp->ds->len = mark.start_offset;
  lp->ds->buf[lp->ds->len] = '\0';
  lp = lp->next;

  while(lp != mark.end_lp) {
    line_t *next = lp->next;
    line_free(lp);
    lp = next;
    E->row_size--;
  }

  dscat(mark.start_lp->ds, lp->ds->buf + mark.end_offset);
  line_free(lp);
  E->row_size--;
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
  usize rows_size = 0;

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
    line_t* line = lalloc(len);
    dscat(line->ds, buffer);

    if (lp_tail == NULL) {
      E->lines = line;
      lp_tail = line;
      continue;
    }

    line->prev = lp_tail;
    lp_tail->next = line;
    lp_tail = line;
  }

  fclose(fp);

  E->row_size = rows_size;
  strcpy(E->filename, filename);

  if (rows_size == 0) editor_new_file(filename, E);
  E->new_file = false;

  return 0;
}

editor_t editor_init(const char *filename) {
  editor_t E = {0};

  if (IO_FILE_EXISTS(filename)) {
    editor_open_file(filename, &E);
  } else {
    editor_new_file(filename, &E);
  }

  return E;
}
