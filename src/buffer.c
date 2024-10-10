#include "buffer.h"

#include <stdlib.h>
#include <string.h>

#include "cursor.h"
#include "editor.h"
#include "globals.h"
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
  bufferl_t *bufl;
  if ((bufl = malloc(sizeof(bufferl_t))) == NULL) DIE("OUT OF MEMMORY");
  if ((bufl->buffer = malloc(sizeof(buffer_t))) == NULL) DIE("OUT OF MEMMORY");
  if ((bufl->buffer->editor = malloc(sizeof(editor_t))) == NULL) DIE("OUT OF MEMMORY");
  if ((bufl->buffer->cursor = malloc(sizeof(cursor_t))) == NULL) DIE("OUT OF MEMMORY");

  editor_init(bufl->buffer->editor, filename);
  cursor_init(bufl->buffer->cursor);
  buffer_set_name(bufl->buffer, filename);

  bufl->buffer->lp = bufl->buffer->editor->lines;
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

buffer_t *buffer_next(void) {
  head = head->next;
  g_window.buffer = head->buffer;
  return head->buffer;
}

buffer_t *buffer_prev(void) {
  head = head->prev;
  g_window.buffer = head->buffer;
  return head->buffer;
}

void buffer_free(void) {
  bufferl_t *aux = head;

  if (head == head->next && head == head->prev) {
    exit(0);
  }

  head->next->prev = head->prev;
  head->prev->next = head->next;
  aux = head;
  head = head->next;

  g_window.buffer = head->buffer;

  cursor_free(aux->buffer->cursor);
  editor_free(aux->buffer->editor);
  free(aux);
}
