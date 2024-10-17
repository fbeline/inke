#include "ifunc.h"


#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "buffer.h"
#include "cursor.h"
#include "definitions.h"
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
  buffer_t *bp = NULL;
  const char *answer = prompt_text();
  switch (answer[0]) {
    case 'y':
      buffer_save(buffer_get());
      break;
    case 'Y':
      bp = buffer_save_all();
      if (bp != NULL) {
        mode_clean();
        set_status_message("Error: Could not save file %.20s", bp->filename);
        return;
      }
      break;
    case 'n':
      break;
  }
  g_flags &= ~RUNNING;
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
  u16 ndirty = buffer_dirty_count();
  switch (ndirty) {
    case 0:
      g_flags &= ~RUNNING;
      return;
    case 1:
      prompt_init("Save file? (y | n): ");
      break;
    default:
      prompt_init("Save modified files? (Y[All] | y[Current] | n[discard]): ");
  }

  g_flags &= ~MINSERT;
  g_flags |= MCMD;
  g_cmd_func = save_and_exit;
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