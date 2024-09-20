#include "globals.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

bool g_running = true;
bool g_cursor_vis = true;
u8 g_mode = MODE_INSERT;
cmd_func_t g_cmd_func = NULL;

static char message[256] = {0};

void set_status_message(const char* msg, ...) {
  va_list args;
	va_start(args, msg);
  vsnprintf(message, 256, msg, args);
	va_end(args);
}

char *get_status_message(void) {
  return message;
}
