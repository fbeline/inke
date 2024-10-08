#include "buffer.h"

#include "stdlib.h"

#include "cursor.h"
#include "editor.h"
#include "utils.h"

typedef struct bufferl_s {
  struct bufferl_s *next;
  struct bufferl_s *prev;

  buffer_t *buffer;
} bufferl_t;

static bufferl_t *head = NULL;

void buffer_create(const char *filename) {
  bufferl_t *bufl;
  if ((bufl = malloc(sizeof(bufferl_t))) == NULL) DIE("OUT OF MEMMORY");
  if ((bufl->buffer = malloc(sizeof(buffer_t))) == NULL) DIE("OUT OF MEMMORY");
  if ((bufl->buffer->editor = malloc(sizeof(editor_t))) == NULL) DIE("OUT OF MEMMORY");
  if ((bufl->buffer->cursor = malloc(sizeof(cursor_t))) == NULL) DIE("OUT OF MEMMORY");

  editor_init(bufl->buffer->editor, filename);
  cursor_init(bufl->buffer->cursor);

  bufl->buffer->cursor->clp = bufl->buffer->editor->lines;

  if (head == NULL) {
    bufl->next = NULL;
    bufl->prev = NULL;
    head = bufl;
  } else {
    bufl->prev = head->prev;
    bufl->next = head;
    head = bufl;
  }
}

buffer_t *buffer_get(void) {
  return head->buffer;
}

buffer_t *buffer_next(void) {
  return NULL;
}

buffer_t *buffer_prev(void) {
  return NULL;
}

void buffer_free(void) {
  bufferl_t *aux = head;
  head = aux->next;
  head->prev = aux->prev;

  cursor_free(aux->buffer->cursor);
  editor_free(aux->buffer->editor);
  free(aux);
}
