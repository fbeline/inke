#include "globals.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

u32 g_flags = (RUNNING | UNDO | MINSERT | CURSORVIS);
ds_t *g_clipbuf = NULL;
cmd_func_t g_cmd_func = NULL;
cmd_func_t g_cmd_complete_func = NULL;
mark_t g_mark = {0};
isearch_t g_isearch = {0};
replace_t g_replace = {0};
window_t g_window = {0};

static char message[256] = {0};

void set_status_message(const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  vsnprintf(message, 256, msg, args);
  va_end(args);
}

const char *get_status_message(void) {
  return message;
}

void mark_start(buffer_t *B) {

  if (g_flags & MVISUAL) {
    g_flags &= ~MVISUAL;
    g_flags |= MINSERT;
    set_status_message("");
    return;
  }

  g_flags &= ~MINSERT;
  g_flags |= MVISUAL;
  set_status_message("visual mode");

  g_mark.start_lp = B->lp;
  g_mark.start_cursor = B->cursor;
  g_mark.start_offset = B->cursor.x + B->cursor.coloff;

  g_mark.end_lp = g_mark.start_lp;
  g_mark.end_cursor = g_mark.start_cursor;
  g_mark.end_offset = g_mark.start_offset;
}

static i8 line_compare(line_t *src, line_t *target) {
  if (src == target) return 0;

  while(target != NULL) {
    if (target == src) return -1;
    target = target->next;
  }

  return 1;
}

void mark_end(buffer_t *B) {
  g_mark.end_lp = B->lp;
  g_mark.end_cursor = B->cursor;
  g_mark.end_offset = B->cursor.x + B->cursor.coloff;
}

mark_t mark_get(void) {
  i8 cmp = line_compare(g_mark.end_lp, g_mark.start_lp);
  if (cmp < 0 || (cmp == 0 && g_mark.start_offset <= g_mark.end_offset)) {
    return g_mark;
  }

  return (mark_t) {
    .start_lp = g_mark.end_lp,
    .end_lp = g_mark.start_lp,
    .end_offset = g_mark.start_offset,
    .start_offset = g_mark.end_offset,
    .start_cursor = g_mark.end_cursor,
    .end_cursor = g_mark.start_cursor
  };
}

void globals_init(void) {
  g_clipbuf = dsempty();
}
