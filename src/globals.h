#pragma once

#include "types.h"

extern bool g_running;
extern bool g_cursor_vis;
extern u8 g_mode;
extern cmd_func_t g_cmd_func;
extern char g_clipbuf[CLIPBUF];
extern mark_t g_mark;

void set_status_message(const char* msg, ...);
void clear_status_message(void);
char *get_status_message(void);

void mark_start(cursor_t *cursor);
void mark_end(cursor_t *cursor);
mark_t mark_get(void);
