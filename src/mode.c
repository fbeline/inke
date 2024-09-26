#include "mode.h"

#include <stdarg.h>

#include "types.h"
#include "globals.h"
#include "io.h"

static void mode_cmd_nop(cursor_t *C, int ch) { }

void mode_cmd_clean(void) {
  set_status_message("");
  g_cursor_vis = true;
  g_mode = MODE_INSERT;
  g_cmd_func = mode_cmd_nop;
}

static void mode_exit_save(cursor_t* C, int ch) {
  switch (ch) {
    case 'y':
      mode_cmd_clean();

      if (io_write_buffer(C->editor) == 0) g_running = false;
      else set_status_message("Error: Could not save file %.20s", C->editor->filename);

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
  set_status_message("Save file? (y/n or [c]ancel)");
  g_cursor_vis = false;
  g_mode = MODE_CMD_CHAR;
  g_cmd_func = mode_exit_save;
}

static void mode_cmd_ctrl_x(cursor_t* C, int ch) {
  if (ch == (CONTROL | 'C')) {
    if (C->editor->dirty) mode_set_exit_save(C);
    else g_running = false;
  } else if (ch == (CONTROL | 'S')) {
    mode_cmd_clean();
    if (io_write_buffer(C->editor) == 0) g_running = false;
    else set_status_message("Error: Could not save file %.20s", C->editor->filename);
  }
}

void mode_set_ctrl_x(cursor_t *C) {
  set_status_message("C-x");
  g_cursor_vis = false;
  g_mode = MODE_CMD_CHAR;
  g_cmd_func = mode_cmd_ctrl_x;
}

