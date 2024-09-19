#include "mode.h"

#include <stdarg.h>
#include <stdio.h>

#include "types.h"
#include "globals.h"
#include "io.h"

static char message[256] = {0};

void mode_set_message(const char* msg, ...) {
  va_list args;
	va_start(args, msg);
  vsnprintf(message, 256, msg, args);
	va_end(args);
}

char* mode_get_message(void) {
  return message;
}

static void mode_cmd_nop(cursor_t *C, int ch) { }

static void mode_cmd_not_found(cursor_t* C, int ch) {
  mode_set_message("keybind not defined");
  g_mode = MODE_INSERT;
}

void mode_cmd_clean(void) {
  mode_set_message("");
  g_mode = MODE_INSERT;
  g_cmd_func = mode_cmd_nop;
}

static void mode_exit_save(cursor_t* C, int ch) {
  char *buf;
  switch (ch) {
    case 'y':
      io_write_buffer(C->editor);
      g_running = false;
      break;
    case 'n':
      g_running = false;
      break;
    case 'c':
      mode_cmd_clean();
      break;
  }
}

void mode_set_exit_save(cursor_t* C) {
  mode_set_message("Save file? (y/n or [c]ancel)");
  g_mode = MODE_CMD_CHAR;
  g_cmd_func = mode_exit_save;
}

static void mode_cmd_ctrl_x(cursor_t* C, int ch) {
  if (ch == (CONTROL | 'C')) {
    if (C->editor->dirty) mode_set_exit_save(C);
    else g_running = false;
  } else if (ch == (CONTROL | 'S')) {
    mode_cmd_clean();
    io_write_buffer(C->editor);
  }
}

void mode_set_ctrl_x(cursor_t *C) {
  mode_set_message("C-x");
  g_mode = MODE_CMD_CHAR;
  g_cmd_func = mode_cmd_ctrl_x;
}

