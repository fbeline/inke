#include "ifunc.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "cursor.h"
#include "definitions.h"
#include "globals.h"
#include "prompt.h"
#include "utils.h"

static void clean_flags(void) {
  set_status_message("");
  g_flags &= ~(MCMD | MSEARCH | CONTROL_X);
  g_flags |= (MINSERT | CURSORVIS);
  g_cmd_func = NULL;
  g_cmd_complete_func = NULL;
}

static void buffer_saving_error(buffer_t *bp) {
  clean_flags();
  set_status_message("Error: Could not save file %.20s", bp->filename);
}

static void save_and_exit(void) {
  buffer_t *bp = NULL;
  const char *answer = prompt_text();
  switch (answer[0]) {
    case 'y':
    case 'Y':
      if (buffer_save(buffer_get()) != 0) {
        buffer_saving_error(buffer_get());
        return;
      }
      break;
    case 'a':
    case 'A':
      bp = buffer_save_all();
      if (bp != NULL) {
        buffer_saving_error(bp);
        return;
      }
      break;
    case 'n':
    case 'N':
      break;
  }
  g_flags &= ~RUNNING;
}

static void kill_current_buffer(void) {
  const char *answer = prompt_text();
  buffer_t *B = buffer_get();

  if (answer[0] == 'y' && buffer_save(B) != 0) {
    buffer_saving_error(B);
    return;
  }

  buffer_free(B);
  clean_flags();
}

static void open_file(void) {
  const char *filename = prompt_text();
  if (access(filename, F_OK) != 0) return;
  buffer_create(filename);
  clean_flags();
}

static void gotol(void) {
  const char *snum = prompt_text();
  i32 line;
  if (sscanf(snum, "%d", &line) == 1) {
    cursor_goto(g_window.buffer, 0, line);
    clean_flags();
  }
}

void set_ctrl_x(buffer_t *B) {
  if (!(g_flags & MINSERT)) return;

  set_status_message("C-x");
  g_flags &= ~(MINSERT | CURSORVIS);
  g_flags |= CONTROL_X;
}

void ifunc_kill_buffer(buffer_t *B) {
  if (!B->dirty) {
    buffer_free(B);
    clean_flags();
    return;
  }

  prompt_init("Save buffer? [Y]YES [N]NO: ");
  g_flags &= ~MINSERT;
  g_flags |= MCMD;
  g_cmd_func = kill_current_buffer;
  g_cmd_complete_func = NULL;
}

void ifunc_save_buffer(buffer_t *B) {
  if (buffer_save(B) != 0) {
    buffer_saving_error(buffer_get());
  }
}

void ifunc_exit(buffer_t *B) {
  u16 ndirty = buffer_dirty_count();
  if (ndirty == 0) {
    g_flags &= ~RUNNING;
    return;
  } else if (ndirty == 1 && B->dirty > 0) {
    prompt_init("Save buffer? [Y]YES [N]NO: ");
  } else if (ndirty > 0) {
    prompt_init("Save modified buffers? [Y]YES [A]YES TO ALL [N]NO: ");
  }

  g_flags &= ~MINSERT;
  g_flags |= MCMD;
  g_cmd_func = save_and_exit;
  g_cmd_complete_func = NULL;
}

void ifunc_find_file(buffer_t *B) {
  g_flags &= ~MINSERT;
  g_flags |= MCMD;
  g_cmd_func = open_file;
  g_cmd_complete_func = prompt_fs_completion;

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
  g_cmd_complete_func = NULL;
}
