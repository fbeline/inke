#include "buffer.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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

static bool buffer_exists(const char *filename) {
  if (head == NULL) return false;

  bufferl_t *blp = head;
  do {
    if (strcmp(filename, blp->buffer->filename) == 0) {
      head = blp;
      g_window.buffer = head->buffer;
      return true;
    }
    blp = blp->next;
  } while(blp != head);

  return false;
}

void buffer_create(const char *filename) {
  char absolute_path[NPATH];
  struct stat file_info;

  if (realpath(filename, absolute_path) == NULL ||
    stat(absolute_path, &file_info) != 0 || !S_ISREG(file_info.st_mode)) {

    if (buffer_get() != NULL)
      set_status_message("Error openning file path %s", filename);
    else
      DIE("Error openning file path %s", filename);

    return;
  }

  if (buffer_exists(absolute_path)) return;

  bufferl_t *bufl;
  if ((bufl = malloc(sizeof(bufferl_t))) == NULL) DIE("OUT OF MEMMORY");
  if ((bufl->buffer = malloc(sizeof(buffer_t))) == NULL) DIE("OUT OF MEMMORY");

  editor_init(&bufl->buffer->editor, absolute_path);
  cursor_init(&bufl->buffer->cursor);
  buffer_set_name(bufl->buffer, absolute_path);

  memcpy(bufl->buffer->filename, absolute_path, NPATH);

  bufl->buffer->lp = bufl->buffer->editor.lines;
  bufl->buffer->dirty = 0;
  bufl->buffer->up = NULL;

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
  g_flags &= ~(MCMD | CONTROL_X);
  g_flags |= MINSERT | CURSORVIS;
  set_status_message("");
}

void buffer_prev(buffer_t *B) {
  head = head->prev;
  g_window.buffer = head->buffer;
  g_flags &= ~(MCMD | CONTROL_X);
  g_flags |= MINSERT | CURSORVIS;
  set_status_message("");
}

void buffer_save(buffer_t *B) {
  if (io_write_buffer(B) != 0) {
    set_status_message("Error: Could not save file %.20s", B->filename);
    g_flags &= ~(MCMD | CONTROL_X);
    g_flags |= MINSERT | CURSORVIS;
    return;
  }

  cursor_t *C = &B->cursor;
  if (C->x + C->coloff > B->lp->ds->len) {
    cursor_eol(B);
  }

  g_flags &= ~(MCMD | CONTROL_X);
  g_flags |= MINSERT | CURSORVIS;
  B->dirty = 0;
  set_status_message("");
}

buffer_t *buffer_save_all(void) {
  bufferl_t *bp = head;
  do {
    if (bp->buffer->dirty > 0) {
      if (io_write_buffer(bp->buffer) != 0)
        return bp->buffer;
    }

    bp->buffer->dirty = 0;
    bp = bp->next;
  } while (bp != head);

  return NULL;
}

u16 buffer_dirty_count(void) {
  bufferl_t *bp = head;
  u16 n = 0;

  do {
    if (bp->buffer->dirty > 0) n++;
    bp = bp->next;
  } while(bp != head);

  return n;
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

  if (head == NULL)
    g_flags &= ~RUNNING;
}
