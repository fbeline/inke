#pragma once

#include "types.h"
#include "ds.h"

#define VERSION "0.0.1"

extern bool g_running;
extern u32 g_mode;
extern u8 g_undo_state;
extern cmd_func_t g_cmd_func;
extern ds_t *g_clipbuf;
extern mark_t g_mark;
extern isearch_t g_isearch;
extern window_t g_window;

void globals_init(void);

void set_status_message(const char* msg, ...);
char *get_status_message(void);

void mark_start(buffer_t *B);
void mark_end(buffer_t *B);
mark_t mark_get(void);
