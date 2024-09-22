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

  capacity = (capacity + BLOCK_SIZE) & ~(BLOCK_SIZE - 1);
  if (capacity == 0)
    capacity = BLOCK_SIZE;

  if ((lp = (line_t*)malloc(sizeof(line_t))) == NULL)
    die("LALLOC OUT OF MEMORY");

  if ((lp->text = malloc(capacity)) == NULL)
    die("LALLOC OUT OF MEMORY");

  lp->capacity = capacity;
  lp->size = 0;
  lp->text[0] = '\0';
  lp->next = NULL;
  lp->prev = NULL;

  return lp;
}

line_t *lrealloc(line_t *lp, usize capacity) {
  capacity = (capacity + BLOCK_SIZE) & ~(BLOCK_SIZE - 1);
  if (capacity == 0)
    capacity = BLOCK_SIZE;

  if ((lp->text = realloc(lp->text, capacity)) == NULL)
    die("LREALLOC OUT OF MEMORY");

  lp->capacity = capacity;

  return lp;
}

line_t* line_append_s(line_t *lp, const char *str, usize len) {
  usize old_size = lp->size;
  usize new_size = lp->size + len;
  if (new_size >= lp->capacity)
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
  if (lp == NULL) return;

  if (lp->prev != NULL) lp->prev->next = lp->next;
  if (lp->next != NULL) lp->next->prev = lp->prev;

  free(lp->text);
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

void editor_delete_lines(editor_t *E, line_t* lp, i32 size) {
  line_t *lp1, *lp2 = lp;
  i32 i = 0;
  do {
    lp1 = lp2;
    lp2 = lp1->next;

    // last line
    if (lp1->prev == NULL && lp1->next == NULL) {
      lp1->text[0] = '\0';
      lp1->size = 0;
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

line_t *editor_rows_to_string(line_t *head, unsigned int size) {
  line_t* ab = lalloc(0);
  line_t* lp = head;
  while (lp != NULL) {
    line_append(ab, lp->text);
    line_append(ab, "\n");
    lp = lp->next;
  }

  return ab;
}

line_t* editor_insert_row_at(editor_t *E, usize y) {
  line_t *pl = NULL;
  line_t *nl = E->lines;

  int i = 0;
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

line_t* editor_insert_row_with_data_at(editor_t *E, usize y, char* strdata) {
  line_t* lp = editor_insert_row_at(E, y);
  line_append(lp, strdata);

  return lp;
}

char editor_char_at(line_t *lp, i32 x) {
  if (lp == NULL || x > lp->size)
    return '\0';

  return lp->text[x];
}

line_t *editor_move_line_up(editor_t *E, line_t *lp) {
  if (lp == NULL || lp->prev == NULL) return NULL;

  line_t *prev = line_append(lp->prev, lp->text);
  line_free(lp);

  E->dirty = true;
  E->row_size--;

  return prev;
}

void editor_delete_char_at(line_t *lp, i32 x) {
  if (x < 0 || x >= lp->size || lp == NULL) return;

  memmove(&lp->text[x],
          &lp->text[x+1],
          lp->size - x + 1);

  lp->size--;
}

line_t *editor_insert_char_at(editor_t *E, line_t *lp, i32 x, char ch) {
  if (x < 0 || x > lp->size)
    die("Invalid position x=%d", x);

  lp->size++;
  if (lp->size >= lp->capacity) lp = lrealloc(lp, lp->size);

  memmove(lp->text + x + 1, lp->text + x, lp->size - x - 1);
  lp->text[x] = ch;
  lp->text[lp->size] = '\0';

  E->dirty = true;

  return lp;
}

void editor_break_line(editor_t *E, line_t *lp, i32 x) {
  if (lp == NULL || x > lp->size) return;

  line_t *new_line = lalloc(16);
  line_t *next_line = lp->next;

  line_append(new_line, lp->text + x);
  lp->text[x] = '\0';
  lp->size = x;

  if (next_line != NULL) next_line->prev = new_line;
  lp->next = new_line;
  new_line->prev = lp;
  new_line->next = next_line;

  E->dirty = true;
  E->row_size++;
}

void editor_delete_forward(line_t *lp, i32 x) {
  lp->text[x] = '\0';
  lp->size = x;
}

void editor_delete_backward(line_t *lp, i32 x) {
  lp->size -= x;
  memmove(lp->text, lp->text + x, lp->size);
  lp->text[lp->size] = '\0';
}

void editor_text_between(editor_t *E, mark_t mark, ds_t *r) {
  r->buf[0] = '\0';
  r->len = 0;

  if (mark.start_lp == mark.end_lp) {
    dsncat(r, 
           mark.start_lp->text + mark.start_offset,
           mark.end_offset - mark.start_offset);
    return;
  }

  line_t *lp = mark.start_lp;
  dscat(r, lp->text + mark.start_offset);
  dscat(r, "\n");
  lp = lp->next;

  while(lp != mark.end_lp && lp != NULL) {
    dscat(r, lp->text);
    dscat(r, "\n");
    lp = lp->next;
  }

  dsncat(r, lp->text, mark.end_offset);
}

void editor_kill_between(editor_t *E, mark_t mark, ds_t *r) {
  line_t *lp = mark.start_lp;

  editor_text_between(E, mark, r);

  if (lp == mark.end_lp) {
    i32 mark_size = mark.end_offset - mark.start_offset;

    memmove(lp->text + mark.start_offset,
            lp->text + mark.end_offset,
            lp->size - mark.end_offset);
    lp->size -= mark_size;
    lp->text[lp->size] = '\0';
    return;
  }

  lp->size = mark.start_offset;
  lp->text[lp->size] = '\0';
  lp = lp->next;

  while(lp != mark.end_lp) {
    line_t *next = lp->next;
    line_free(lp);
    lp = next;
  }

  line_append(mark.start_lp, lp->text + mark.end_offset);
  line_free(lp);
}

void editor_insert_text(editor_t *E, line_t* lp, i32 x, const char* strdata) {
  if (x < 0 || strdata == NULL || lp == NULL) return;
  x = MIN(x, (i32)lp->size);

  char* strtail = strdup(lp->text + x);
  lp->text[x] = '\0';
  lp->size = x;

  i32 i = 0, j = 0;
  char aux[CLIPBUF];
  char ch;
  while ((ch = strdata[i++]) != '\0') {
    if (ch != '\n') {
      aux[j++] = ch;
      continue;
    }

    aux[j] = '\0';
    line_append(lp, aux);
    j = 0;
    aux[0] = '\0';
    lp = editor_create_line_after(E, lp, 16);
  }
  aux[j] = '\0';
  line_append(lp, aux);
  line_append(lp, strtail);
  free(strtail);
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
    line_append(line, buffer);

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
