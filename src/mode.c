#include "mode.h"

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "buffer.h"
#include "cursor.h"
#include "cmdline.h"
#include "globals.h"
#include "isearch.h"
#include "io.h"
#include "utils.h"

static void mode_cmd_nop(int ch) { }

void mode_cmd_clean(void) {
  clear_status_message();
  g_cursor_vis = true;
  g_mode = MODE_INSERT;
  g_cmd_func = mode_cmd_nop;
}

static void mode_exit_save(int ch) {
  switch (ch) {
    case 'y':
      mode_cmd_clean();
      editor_t *E = buffer_get()->editor;
      if (io_write_buffer(E) == 0) g_running = false;
      else set_status_message("Error: Could not save file %.20s", E->filename);

      break;
    case 'n':
      g_running = false;
      break;
    case 'c':
      mode_cmd_clean();
      break;
  }
}

void mode_set_exit_save() {
  set_status_message("Save file? (y/n or [c]ancel)");
  g_cursor_vis = false;
  g_mode = MODE_CMD_CHAR;
  g_cmd_func = mode_exit_save;
}

static void mode_cmd_open_file(i32 _ch) {
  const char *filename = cmdline_text();
  if (access(filename, F_OK) != 0) return;
  buffer_create(filename);
  mode_cmd_clean();
}

static void mode_set_find_file(void) {
  g_mode = MODE_CMD;
  g_cmd_func = mode_cmd_open_file;

  char cwd[NPATH];
  if (getcwd(cwd, sizeof(cwd)) == NULL) DIE("Error getcwd");

  cmdline_init("Find file: ");
  cmdline_cat(cwd);
  cmdline_insert('/');
}

static void mode_cmd_ctrl_x(int ch) {
  editor_t *E = buffer_get()->editor;
  if (ch == (CONTROL | 'C')) {
    if (E->dirty) mode_set_exit_save();
    else g_running = false;
  } else if (ch == (CONTROL | 'F')) {
    mode_set_find_file();
  } else if (ch == (CONTROL | 'S')) {
    mode_cmd_clean();

    if (io_write_buffer(E) != 0)
      set_status_message("Error: Could not save file %.20s", E->filename);

    cursor_t *C = buffer_get()->cursor;
    if (C->x + C->coloff > C->clp->ds->len) {
      cursor_eol(C);
    }
  }
}

static void mode_cmd_gotol(int ch) {
  cursor_t *C = buffer_get()->cursor;
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
