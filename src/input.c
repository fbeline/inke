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
#include "types.h"
#include "utils.h"

static keytab_t keytabs[] = {
  { (MODE_INSERT | MODE_VISUAL), HOME_KEY, cursor_bol },
  { (MODE_INSERT | MODE_VISUAL), END_KEY, cursor_eol },
  { (MODE_INSERT | MODE_VISUAL), PAGE_DOWN, cursor_page_down },
  { (MODE_INSERT | MODE_VISUAL), PAGE_UP, cursor_page_up },
  { (MODE_INSERT | MODE_VISUAL), ARROW_LEFT, cursor_left },
  { (MODE_INSERT | MODE_VISUAL), ARROW_RIGHT, cursor_right },
  { (MODE_INSERT | MODE_VISUAL), ARROW_UP, cursor_up },
  { (MODE_INSERT | MODE_VISUAL), ARROW_DOWN, cursor_down },
  { (MODE_INSERT | MODE_VISUAL), META | 'f', cursor_move_word_forward },
  { (MODE_INSERT | MODE_VISUAL), META | 'b', cursor_move_word_backward },
  { (MODE_INSERT | MODE_VISUAL), META | '^', cursor_move_line_up },
  { (MODE_INSERT | MODE_VISUAL), META | '>', cursor_eof },
  { (MODE_INSERT | MODE_VISUAL), META | '<', cursor_bof },
  { (MODE_INSERT | MODE_VISUAL), CONTROL | ' ', mark_start },
  { (MODE_INSERT | MODE_VISUAL), CONTROL | 'N', cursor_down },
  { (MODE_INSERT | MODE_VISUAL), CONTROL | 'P', cursor_up },
  { (MODE_INSERT | MODE_VISUAL), CONTROL | 'A', cursor_bol },
  { (MODE_INSERT | MODE_VISUAL), CONTROL | 'B', cursor_left },
  { (MODE_INSERT | MODE_VISUAL), CONTROL | 'E', cursor_eol },
  { (MODE_INSERT | MODE_VISUAL), CONTROL | 'F', cursor_right },

  { MODE_VISUAL, CONTROL | 'W', cursor_region_kill },
  { MODE_VISUAL, META | 'W', cursor_region_text },

  { MODE_INSERT, BACKSPACE_KEY, cursor_remove_char },
  { MODE_INSERT, DEL_KEY, cursor_remove_char },
  { MODE_INSERT, ENTER_KEY, cursor_break_line },
  { MODE_INSERT, META | 'g', ifunc_gotol },
  { MODE_INSERT, CONTROL | 'H', cursor_remove_char },
  { MODE_INSERT, CONTROL | 'K', cursor_delete_forward },
  { MODE_INSERT, CONTROL | 'S', isearch_start },
  { MODE_INSERT, CONTROL | 'X', set_ctrl_x },
  { MODE_INSERT, CONTROL | 'Y', cursor_paste },
  { MODE_INSERT, CONTROL | '/', cursor_undo },

  { CONTROL_X, 'K', buffer_free },
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
    if ((ktp->flags & g_mode) && ktp->code == c)
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

  if (g_mode != MODE_INSERT) return;

  if (ch == TAB_KEY) {
    cursor_insert_char(B, ' ');
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
    if (g_mode == MODE_SEARCH)
      isearch_abort(B);

    g_mode = MODE_INSERT;
    set_status_message("Quit");
    return;
  }

  switch (g_mode) {
    case MODE_CMD_CHAR:
      g_cmd_func(ch);
      break;
    case MODE_SEARCH:
    case MODE_CMD:
      prompt_handle_char(ch);
      break;
    default:
      process_key(B, ch);
  }

  if (g_mode == MODE_VISUAL)
    mark_end(B);
}
