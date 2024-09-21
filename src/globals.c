#include "globals.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

bool g_running = true;
bool g_cursor_vis = true;
u8 g_mode = MODE_INSERT;
cmd_func_t g_cmd_func = NULL;
mark_t g_mark = {0};

char g_clipbuf[CLIPBUF] = {0};

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

  i8 cmp = line_compare(C->clp, g_mark.start_lp);
  if (cmp < 0 || (cmp == 0 && g_mark.start_offset <= C->x + C->coloff)) {
    g_mark.end_lp = C->clp;
    g_mark.end_offset = C->x + C->coloff;
    return;
  }

  // TODO : FIX IT
  g_mark.start_lp = C->clp;
  g_mark.start_offset = C->x + C->coloff;
}
