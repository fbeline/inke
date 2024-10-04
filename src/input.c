#include "input.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "cmdline.h"
#include "cursor.h"
#include "globals.h"
#include "isearch.h"
#include "mode.h"
#include "types.h"
#include "utils.h"

static keytab_t keytabs[NBINDS] = {
  { HOME_KEY, cursor_bol },
  { END_KEY, cursor_eol },
  { PAGE_DOWN, cursor_page_down },
  { PAGE_UP, cursor_page_up },
  { ARROW_LEFT, cursor_left },
  { ARROW_RIGHT, cursor_right },
  { ARROW_UP, cursor_up },
  { ARROW_DOWN, cursor_down },
  { BACKSPACE_KEY, cursor_remove_char },
  { DEL_KEY, cursor_remove_char },
  { ENTER_KEY, cursor_break_line },

  { CONTROL | ' ', mark_start },
  { CONTROL | 'A', cursor_bol },
  { CONTROL | 'B', cursor_left },
  { CONTROL | 'E', cursor_eol },
  { CONTROL | 'F', cursor_right },
  { CONTROL | 'H', cursor_remove_char },
  { CONTROL | 'K', cursor_delete_forward },
  { CONTROL | 'N', cursor_down },
  { CONTROL | 'P', cursor_up },
  { CONTROL | 'S', isearch_start },
  { CONTROL | 'X', mode_set_ctrl_x },
  { CONTROL | 'Y', cursor_paste },
  { CONTROL | '/', cursor_undo },

  { META | 'f', cursor_move_word_forward },
  { META | 'b', cursor_move_word_backward },
  { META | '^', cursor_move_line_up },
  { META | '>', cursor_eof },
  { META | '<', cursor_bof },

  { 0, NULL}
};

static keytab_t keytabs_visual[NBINDS] = {
  { HOME_KEY, cursor_bol },
  { END_KEY, cursor_eol },
  { PAGE_DOWN, cursor_page_down },
  { PAGE_UP, cursor_page_up },
  { ARROW_LEFT, cursor_left },
  { ARROW_RIGHT, cursor_right },
  { ARROW_UP, cursor_up },
  { ARROW_DOWN, cursor_down },

  { CONTROL | ' ', mark_start },
  { CONTROL | 'A', cursor_bol },
  { CONTROL | 'B', cursor_left },
  { CONTROL | 'E', cursor_eol },
  { CONTROL | 'F', cursor_right },
  { CONTROL | 'N', cursor_down },
  { CONTROL | 'P', cursor_up },
  { CONTROL | 'W', cursor_region_kill },

  { META | 'f', cursor_move_word_forward },
  { META | 'b', cursor_move_word_backward },
  { META | 'w', cursor_region_text },
  { META | '>', cursor_eof },
  { META | '<', cursor_bof },

  { 0, NULL}
};

static key_func_t get_kfp(keytab_t *keytabs, int c) {
	keytab_t *ktp = &keytabs[0];

	while (ktp->fp != NULL) {
		if (ktp->code == c)
			return ktp->fp;
		++ktp;
	}

	return NULL;
}

static i32 input_read_key(void) {
  i32 nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) DIE("read");
  }

  if (c == '\x1b') {
    char seq[3];

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
    if (c == 0x1F)
      return CONTROL | '/';

    if (c == 0x00)
      return CONTROL | ' ';

    return CONTROL | (c + '@');
  }

  return c;
}

static void process_insert_mode(cursor_t *C, i32 ch) {
  key_func_t kfp;
  if ((kfp = get_kfp(keytabs, ch)) != NULL) {
    kfp(C);
    return;
  }

  if (ch == TAB_KEY) {
    cursor_insert_char(C, ' ');
    cursor_insert_char(C, ' ');
  } else if (ch >= 32 && ch <= 126) {
    cursor_insert_char(C, ch);
  }
}

static void process_visual_mode(cursor_t *C, i32 ch) {
  key_func_t kfp;
  if ((kfp = get_kfp(keytabs_visual, ch)) != NULL) {
    kfp(C);
    return;
  }
  set_status_message("visual mode - cmd not found");
}

static void process_cmd_mode(cursor_t *C, i32 ch) {
  switch (ch) {
    case ENTER_KEY:
      g_cmd_func(C, 0);
      break;
    case BACKSPACE_KEY:
      cmdline_backspace();
      break;
    case ARROW_LEFT:
      cmdline_left();
      break;
    case ARROW_RIGHT:
      cmdline_right();
      break;
    default:
      if (g_mode == MODE_SEARCH && ch == (CONTROL | 'S')) {
        g_cmd_func(C, 1);
      }
      else if (g_mode == MODE_SEARCH && ch == (CONTROL | 'R')) {
        g_cmd_func(C, -1);
      }
      else if (ch == (CONTROL | 'E')) {
        cmdline_eol();
      }
      else if (ch == (CONTROL | 'A')) {
        cmdline_bol();
      }
      else if (ch == (CONTROL | 'U')) {
        cmdline_del_before();
      } else if (ch >= 32 && ch <= 126) {
        cmdline_insert(ch);
      }
  }
}

void input_process_keys(cursor_t* C) {
  i32 ch = input_read_key();

  if (ch == (CONTROL | 'G')) {
    mode_cmd_clean();
    cmdline_clean();
    set_status_message("Quit");
    return;
  }

  switch (g_mode) {
    case MODE_INSERT:
      process_insert_mode(C, ch);
      break;
    case MODE_VISUAL:
      process_visual_mode(C, ch);
      break;
    case MODE_CMD_CHAR:
      g_cmd_func(C, ch);
      break;
    case MODE_SEARCH:
    case MODE_CMD:
      process_cmd_mode(C, ch);
      break;
  }

  if (g_mode == MODE_VISUAL)
    mark_end(C);
}
