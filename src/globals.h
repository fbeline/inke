#pragma once

#include "definitions.h"
#include "ds.h"

#define VERSION "0.0.1"

extern u32 g_flags;
extern cmd_func_t g_cmd_func;
extern cmd_func_t g_cmd_complete_func;
extern ds_t *g_clipbuf;
extern mark_t g_mark;
extern isearch_t g_isearch;
extern replace_t g_replace;
extern window_t g_window;

void globals_init(void);

void set_status_message(const char* msg, ...);
const char *get_status_message(void);

void mark_start(buffer_t *B);
void mark_end(buffer_t *B);
mark_t mark_get(void);
