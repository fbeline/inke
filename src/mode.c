#include "mode.h"

#include <stdarg.h>
#include <stdio.h>

#include "cmdline.h"
#include "globals.h"
#include "isearch.h"
#include "io.h"
#include "types.h"

static void mode_cmd_nop(cursor_t *C, int ch) { }

void mode_cmd_clean(void) {
  clear_status_message();
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

    if (io_write_buffer(C->editor) != 0)
      set_status_message("Error: Could not save file %.20s", C->editor->filename);

    if (C->x + C->coloff > C->clp->ds->len)
      cursor_eol(C);
  }
}

static void mode_cmd_gotol(cursor_t* C, int ch) {
  const char *snum = cmdline_text();
  i32 line;
  if (sscanf(snum, "%d", &line) == 1) {
    cursor_goto(C, 0, line);
    mode_cmd_clean();
  }
}

void mode_set_ctrl_x(cursor_t *C) {
  set_status_message("C-x");
  g_cursor_vis = false;
  g_mode = MODE_CMD_CHAR;
  g_cmd_func = mode_cmd_ctrl_x;
}

void mode_set_gotol(cursor_t *C) {
  cmdline_init("goto line: ");
  g_mode = MODE_CMD;
  g_cmd_func = mode_cmd_gotol;
}
