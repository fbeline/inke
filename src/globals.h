#pragma once

#include "types.h"

extern bool g_running;
extern bool g_cursor_vis;
extern u8 g_mode;
extern cmd_func_t g_cmd_func;

void set_status_message(const char* msg, ...);
char *get_status_message(void);
