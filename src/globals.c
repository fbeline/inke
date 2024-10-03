#include "globals.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

bool g_running = true;
bool g_cursor_vis = true;
u8 g_mode = MODE_INSERT;
u8 g_undo_state = UNDO_ON;
ds_t *g_clipbuf = NULL;
cmd_func_t g_cmd_func = NULL;
mark_t g_mark = {0};
isearch_t g_isearch = {0};

static char message[256] = {0};

void clear_status_message(void) {
  message[0] = '\0';
}

void set_status_message(const char* msg, ...) {
  va_list args;
	va_start(args, msg);
  vsnprintf(message, 256, msg, args);
	va_end(args);
}

char *get_status_message(void) {
  return message;
}

void mark_start(cursor_t *C) {

  if (g_mode == MODE_VISUAL) {
    g_mode = MODE_INSERT;
    clear_status_message();
    return;
  }

  g_mode = MODE_VISUAL;
  set_status_message("visual mode");

  g_mark.start_lp = C->clp;
  g_mark.start_offset = C->x + C->coloff;

  g_mark.end_lp = g_mark.start_lp;
  g_mark.end_lp = g_mark.start_lp;
}

static i8 line_compare(line_t *src, line_t *target) {
  if (src == target) return 0;

  while(target != NULL) {
    if (target == src) return -1;
    target = target->next;
  }

  return 1;
}

void mark_end(cursor_t *C) {
  g_mark.end_lp = C->clp;
  g_mark.end_offset = C->x + C->coloff;
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
    .start_offset = g_mark.end_offset
  };
}

void globals_init(void) {
  g_clipbuf = dsempty();
}
