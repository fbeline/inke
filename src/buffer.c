#include "buffer.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "cursor.h"
#include "ed_stack.h"
#include "ed_list.h"
#include "editor.h"
#include "globals.h"
#include "io.h"
#include "undo.h"
#include "utils.h"

static struct ed_list *buffers = NULL;

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
  if (buffers == NULL) return false;

  buffer_t *bp = (buffer_t*)buffers;
  do {
    if (strcmp(filename, bp->filename) == 0) {
      buffers = (struct ed_list*)bp;
      g_window.buffer = bp;
      return true;
    }
    bp = bp->next;
  } while(bp != NULL && bp != (buffer_t*)buffers);

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
      DIE("Error openning file: %s", filename);

    return;
  }

  if (buffer_exists(absolute_path)) return;

  buffer_t *bp;
  if ((bp = malloc(sizeof(buffer_t))) == NULL)
    DIE("OUT OF MEMMORY");

  editor_init(&bp->editor, absolute_path);
  cursor_init(&bp->cursor);
  buffer_set_name(bp, absolute_path);

  memcpy(bp->filename, absolute_path, NPATH);

  bp->lp = bp->editor.lines;
  bp->dirty = 0;
  bp->next = NULL;
  bp->prev = NULL;
  bp->undo_stack = NULL;

  if (ed_list_append(&buffers, (struct ed_list*) bp) != 0)
    DIE("BUFFER APPEND ERROR");

  g_window.buffer = bp;
}

buffer_t *buffer_get(void) {
  return g_window.buffer;
}

void buffer_next(buffer_t *B) {
  g_window.buffer = B->next;
  g_flags &= ~(MCMD | CONTROL_X);
  g_flags |= MINSERT | CURSORVIS;
  set_status_message("");
}

void buffer_prev(buffer_t *B) {
  g_window.buffer = B->prev;
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
  buffer_t *bp = (buffer_t*)buffers;
  do {
    if (bp->dirty > 0) {
      if (io_write_buffer(bp) != 0)
        return bp;
    }

    bp->dirty = 0;
    bp = bp->next;
  } while (bp != (buffer_t*)buffers);

  return NULL;
}

u16 buffer_dirty_count(void) {
  buffer_t *bp = (buffer_t*)buffers;
  u16 n = 0;

  do {
    if (bp->dirty > 0) n++;
    bp = bp->next;
  } while(bp != (buffer_t*)buffers);

  return n;
}

void buffer_free(buffer_t *B) {
  if (ed_list_len(&buffers) == 1) {
    exit(0);
  }

  ed_list_remove(&buffers, (struct ed_list*)B);

  struct ed_stack *top = ed_stack_pop(&B->undo_stack);
  while(top != NULL) {
    undo_free((undo_t*)top);
    top = ed_stack_pop(&B->undo_stack);
  }

  editor_free(&B->editor);
  free(B);

  g_window.buffer = (buffer_t*)buffers;
}
