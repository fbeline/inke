#include "ifunc.h"

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "buffer.h"
#include "cursor.h"
#include "globals.h"
#include "io.h"
#include "prompt.h"
#include "utils.h"

static void ifunc_nop(void) { }

static void mode_clean(void) {
  set_status_message("");
  g_flags &= ~(MCMD | MSEARCH | CONTROL_X);
  g_flags |= MINSERT;
  g_cmd_func = ifunc_nop;
}

static void save_and_exit(void) {
  const char *answer = prompt_text();
  switch (answer[0]) {
    case 'y':
      mode_clean();
      if (io_write_buffer(g_window.buffer) == 0) g_flags &= ~RUNNING;
      else set_status_message("Error: Could not save file %.20s",
                              g_window.buffer->filename);

      break;
    case 'n':
      g_flags &= ~RUNNING;
      break;
    case 'c':
      mode_clean();
      break;
  }
}

static void open_file(void) {
  const char *filename = prompt_text();
  if (access(filename, F_OK) != 0) return;
  buffer_create(filename);
  mode_clean();
}

static void gotol(void) {
  const char *snum = prompt_text();
  i32 line;
  if (sscanf(snum, "%d", &line) == 1) {
    cursor_goto(g_window.buffer, 0, line);
    mode_clean();
  }
}

void set_ctrl_x(buffer_t *B) {
  if (!(g_flags & MINSERT)) return;

  set_status_message("C-x");
  g_flags &= ~MINSERT;
  g_flags |= CONTROL_X;
}

void ifunc_exit(buffer_t *B) {
  if (B->dirty > 0) {
    prompt_init("Save file? (y/n or [c]ancel): ");
    g_flags &= ~MINSERT;
    g_flags |= MCMD;
    g_cmd_func = save_and_exit;
  }
  else {
    g_flags &= ~RUNNING;
  }
}

void ifunc_find_file(buffer_t *B) {
  g_flags &= ~MINSERT;
  g_flags |= MCMD;
  g_cmd_func = open_file;

  char cwd[NPATH];
  if (getcwd(cwd, sizeof(cwd)) == NULL) DIE("Error getcwd");

  prompt_init("Find file: ");
  prompt_cat(cwd);
  prompt_insert('/');
}

void ifunc_gotol(buffer_t *B) {
  prompt_init("goto line: ");
  g_flags &= ~MINSERT;
  g_flags |= MCMD;
  g_cmd_func = gotol;
}
