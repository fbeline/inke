#include "mode.h"

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "buffer.h"
#include "cursor.h"
#include "prompt.h"
#include "globals.h"
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
      if (io_write_buffer(g_window.buffer) == 0) g_running = false;
      else set_status_message("Error: Could not save file %.20s",
                              g_window.buffer->filename);

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
  const char *filename = prompt_text();
  if (access(filename, F_OK) != 0) return;
  buffer_create(filename);
  mode_cmd_clean();
}

static void mode_set_find_file(void) {
  g_mode = MODE_CMD;
  g_cmd_func = mode_cmd_open_file;

  char cwd[NPATH];
  if (getcwd(cwd, sizeof(cwd)) == NULL) DIE("Error getcwd");

  prompt_init("Find file: ");
  prompt_cat(cwd);
  prompt_insert('/');
}

static void mode_cmd_ctrl_x(i32 ch) {
  buffer_t *B = g_window.buffer;
  if (ch == (CONTROL | 'C')) {
    if (B->dirty > 0) mode_set_exit_save();
    else g_running = false;
  } else if (ch == (CONTROL | 'F')) {
    mode_set_find_file();
  } else if (ch == (CONTROL | ARROW_RIGHT)) {
    buffer_next();
    mode_cmd_clean();
  } else if (ch == (CONTROL | ARROW_LEFT)) {
    buffer_prev();
    mode_cmd_clean();
  } else if (ch == 'k') {
    buffer_free();
    mode_cmd_clean();
  } else if (ch == (CONTROL | 'S')) {
    mode_cmd_clean();

    if (io_write_buffer(B) != 0)
      set_status_message("Error: Could not save file %.20s", B->filename);

    cursor_t *C = &g_window.buffer->cursor;
    if (C->x + C->coloff > B->lp->ds->len) {
      cursor_eol(B);
    }
  }
}

static void mode_cmd_gotol(i32 ch) {
  const char *snum = prompt_text();
  i32 line;
  if (sscanf(snum, "%d", &line) == 1) {
    cursor_goto(g_window.buffer, 0, line);
    mode_cmd_clean();
  }
}

void mode_set_ctrl_x(buffer_t *B) {
  set_status_message("C-x");
  g_cursor_vis = false;
  g_mode = MODE_CMD_CHAR;
  g_cmd_func = mode_cmd_ctrl_x;
}

void mode_set_gotol(buffer_t *B) {
  prompt_init("goto line: ");
  g_mode = MODE_CMD;
  g_cmd_func = mode_cmd_gotol;
}
