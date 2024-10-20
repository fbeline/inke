#include "input.h"

#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "buffer.h"
#include "cursor.h"
#include "globals.h"
#include "ifunc.h"
#include "isearch.h"
#include "prompt.h"
#include "definitions.h"
#include "utils.h"

static keytab_t keytabs[] = {
  { (MINSERT | MVISUAL), HOME_KEY, cursor_bol },
  { (MINSERT | MVISUAL), END_KEY, cursor_eol },
  { (MINSERT | MVISUAL), PAGE_DOWN, cursor_page_down },
  { (MINSERT | MVISUAL), PAGE_UP, cursor_page_up },
  { (MINSERT | MVISUAL), ARROW_LEFT, cursor_left },
  { (MINSERT | MVISUAL), ARROW_RIGHT, cursor_right },
  { (MINSERT | MVISUAL), ARROW_UP, cursor_up },
  { (MINSERT | MVISUAL), ARROW_DOWN, cursor_down },
  { (MINSERT | MVISUAL), META | 'f', cursor_move_word_forward },
  { (MINSERT | MVISUAL), META | 'b', cursor_move_word_backward },
  { (MINSERT | MVISUAL), META | '^', cursor_move_line_up },
  { (MINSERT | MVISUAL), META | '>', cursor_eof },
  { (MINSERT | MVISUAL), META | '<', cursor_bof },
  { (MINSERT | MVISUAL), META | '}', cursor_move_paragraph_forward },
  { (MINSERT | MVISUAL), META | '{', cursor_move_paragraph_backward },
  { (MINSERT | MVISUAL), CONTROL | ' ', mark_start },
  { (MINSERT | MVISUAL), CONTROL | 'N', cursor_down },
  { (MINSERT | MVISUAL), CONTROL | 'P', cursor_up },
  { (MINSERT | MVISUAL), CONTROL | 'A', cursor_bol },
  { (MINSERT | MVISUAL), CONTROL | 'B', cursor_left },
  { (MINSERT | MVISUAL), CONTROL | 'E', cursor_eol },
  { (MINSERT | MVISUAL), CONTROL | 'F', cursor_right },

  { MVISUAL, CONTROL | 'W', cursor_region_kill },
  { MVISUAL, META | 'w', cursor_region_text },
  { MVISUAL, '>', cursor_move_region_forward },
  { MVISUAL, '<', cursor_move_region_backward },

  { MINSERT, BACKSPACE_KEY, cursor_remove_char },
  { MINSERT, DEL_KEY, cursor_remove_char },
  { MINSERT, ENTER_KEY, cursor_break_line },
  { MINSERT, META | 'g', ifunc_gotol },
  { MINSERT, CONTROL | 'H', cursor_remove_char },
  { MINSERT, CONTROL | 'K', cursor_delete_forward },
  { MINSERT, CONTROL | 'S', isearch_start },
  { MINSERT, CONTROL | 'X', set_ctrl_x },
  { MINSERT, CONTROL | 'Y', cursor_paste },
  { MINSERT, CONTROL | '/', cursor_undo },

  { CONTROL_X, 'k', ifunc_kill_buffer },
  { CONTROL_X, CONTROL | 'F', ifunc_find_file },
  { CONTROL_X, CONTROL | 'S', buffer_save },
  { CONTROL_X, CONTROL | 'C', ifunc_exit },
  { CONTROL_X, CONTROL | ARROW_RIGHT, buffer_next },
  { CONTROL_X, CONTROL | ARROW_LEFT, buffer_prev },

  { 0, 0, NULL}
};

static key_func_t get_kfp(i32 c) {
  keytab_t *ktp = &keytabs[0];

  while (ktp->fp != NULL) {
    if ((ktp->flags & g_flags) && ktp->code == c)
      return ktp->fp;
    ++ktp;
  }

  return NULL;
}

static void process_key(buffer_t *B, i32 ch) {
  key_func_t kfp;
  if ((kfp = get_kfp(ch)) != NULL) {
    kfp(B);
    return;
  }

  if (!(g_flags & MINSERT)) return;

  if (ch == TAB_KEY) {
    for (u8 i = 0; i < TAB_STOP; i++)
      cursor_insert_char(B, ' ');
  } else if (ch >= 32 && ch <= 126) {
    cursor_insert_char(B, ch);
  }
}

static i32 input_read_key(void) {
  i32 nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) DIE("read");
  }

  if (c == '\t')
    return TAB_KEY;

  if (c == '\x1b') {
    char seq[5];

    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';

    if (seq[0] == '[') {
      if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
        if (seq[2] == '~') {
          switch (seq[1]) {
            case '1': return HOME_KEY;
            case '3': return DEL_KEY;
            case '4': return END_KEY;
            case '5': return PAGE_UP;
            case '6': return PAGE_DOWN;
            case '7': return HOME_KEY;
            case '8': return END_KEY;
          }
        } else if (seq[2] == ';') {
          if (read(STDIN_FILENO, &seq[3], 1) != 1) return '\x1b';
          if (read(STDIN_FILENO, &seq[4], 1) != 1) return '\x1b';

          if (seq[3] == '5') {
            switch (seq[4]) {
              case 'A': return CONTROL | ARROW_UP;
              case 'B': return CONTROL | ARROW_DOWN;
              case 'C': return CONTROL | ARROW_RIGHT;
              case 'D': return CONTROL | ARROW_LEFT;
            }
          }
        }
      } else {
        switch (seq[1]) {
          case 'A': return ARROW_UP;
          case 'B': return ARROW_DOWN;
          case 'C': return ARROW_RIGHT;
          case 'D': return ARROW_LEFT;
        }
      }
    } else if (seq[0] == 'O') {
      if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
      switch (seq[1]) {
        case 'H': return HOME_KEY;
        case 'F': return END_KEY;
      }

      return '\x1b';
    } else if (seq[0] >= ' ' && seq[0] <= '~') {
      return META | seq[0];
    }
  }

  if (c >= 0x00 && c <= 0x1F && c != ENTER_KEY) {
    switch (c) {
      case 0x1F:
        return CONTROL | '/';
      case 0x00:
        return CONTROL | ' ';
      default:
        return CONTROL | (c + '@');
    }
  }

  return c;
}

void input_process_keys(buffer_t* B) {
  i32 ch = input_read_key();

  if (ch == (CONTROL | 'G')) {
    if (g_flags & MSEARCH)
      isearch_abort(B);

    g_flags = (RUNNING | UNDO | MINSERT);
    set_status_message("Quit");
    return;
  }

  if (g_flags & (MSEARCH | MCMD)) {
    prompt_handle_char(ch);
  } else {
    process_key(B, ch);
  }

  if (g_flags & MVISUAL)
    mark_end(B);
}
