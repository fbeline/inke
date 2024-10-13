#include "buffer.h"

#include <stdlib.h>
#include <string.h>

#include "cursor.h"
#include "editor.h"
#include "globals.h"
#include "io.h"
#include "utils.h"

typedef struct bufferl_s {
  struct bufferl_s *next;
  struct bufferl_s *prev;

  buffer_t *buffer;
} bufferl_t;

static bufferl_t *head = NULL;

static void buffer_set_name(buffer_t *buffer, const char *filename) {
  usize len = 0;
  const char* name = strrchr(filename, '/');
  if (name) {
    len = MIN(strlen(name + 1), NBUFNAME - 1);
    memcpy(buffer->name, name + 1, len);
  } else {
    len = MIN(strlen(filename), NBUFNAME - 1);
    memcpy(buffer->name, filename, len);
  }
  buffer->name[len] = '\0';
}

void buffer_create(const char *filename) {
  usize fnlen = strlen(filename);
  if (fnlen >= NPATH) return;

  bufferl_t *bufl;
  if ((bufl = malloc(sizeof(bufferl_t))) == NULL) DIE("OUT OF MEMMORY");
  if ((bufl->buffer = malloc(sizeof(buffer_t))) == NULL) DIE("OUT OF MEMMORY");

  editor_init(&bufl->buffer->editor, filename);
  cursor_init(&bufl->buffer->cursor);
  buffer_set_name(bufl->buffer, filename);

  memcpy(bufl->buffer->filename, filename, fnlen);
  bufl->buffer->filename[fnlen] = '\0';

  bufl->buffer->lp = bufl->buffer->editor.lines;
  bufl->buffer->dirty = 0;

  if (head == NULL) {
    bufl->next = bufl;
    bufl->prev = bufl;
    head = bufl;
  } else {
    bufl->prev = head->prev;
    bufl->prev->next = bufl;
    bufl->next = head;
    head->prev = bufl;
    head = bufl;
  }

  g_window.buffer = head->buffer;
}

buffer_t *buffer_get(void) {
  return head->buffer;
}

void buffer_next(buffer_t *B) {
  head = head->next;
  g_window.buffer = head->buffer;
  g_flags = MINSERT;
  set_status_message("");
}

void buffer_prev(buffer_t *B) {
  head = head->prev;
  g_window.buffer = head->buffer;
  g_flags = MINSERT;
  set_status_message("");
}

void buffer_save(buffer_t *B) {
  if (io_write_buffer(B) != 0)
    set_status_message("Error: Could not save file %.20s", B->filename);

  cursor_t *C = &B->cursor;
  if (C->x + C->coloff > B->lp->ds->len) {
    cursor_eol(B);
  }
  g_flags = MINSERT;
  set_status_message("");
}

void buffer_free(buffer_t *B) {
  bufferl_t *aux = head;

  if (head == head->next && head == head->prev) {
    exit(0);
  }

  head->next->prev = head->prev;
  head->prev->next = head->next;
  aux = head;
  head = head->next;

  g_window.buffer = head->buffer;

  editor_free(&aux->buffer->editor);
  free(aux);
}
