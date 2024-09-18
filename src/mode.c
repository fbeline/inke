#include "mode.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "io.h"
#include "utils.h"
#include "vt100.h"

u8 g_mode = MODE_INSERT;
cmd_func_t g_cmd_func = NULL;
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

static void mode_cmd_nop(editor_t* E, char r) { }

static void mode_cmd_not_found(editor_t* E, char r) {
  mode_set_message("keybind not defined");
  g_mode = MODE_INSERT;
}

void mode_cmd_clean(void) {
  mode_set_message("");
  g_mode = MODE_INSERT;
  g_cmd_func = mode_cmd_nop;
  vt_show_cursor();
  tt_flush();
}

static void mode_exit_save(editor_t* E, char r) {
  char *buf;
  switch (r) {
    case 'y':
      io_write_buffer(E);
      E->running = false;
      break;
    case 'n':
      E->running = false;
      break;
    case 'c':
      mode_cmd_clean();
      break;
  }
}

void mode_set_exit_save(editor_t* E) {
  vt_hide_cursor();
  tt_flush();
  mode_set_message("Save file? (y/n or [c]ancel)");
  g_mode = MODE_CMD_CHAR;
  g_cmd_func = mode_exit_save;
}

void mode_set_ctrl_x(void) {
  mode_set_message("C-x");
  g_mode = MODE_CMD;
  g_cmd_func = mode_cmd_not_found;
}

