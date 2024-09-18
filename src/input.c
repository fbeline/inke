#include "input.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "cursor.h"
#include "mode.h"
#include "utils.h"
#include "vt100.h"

#define NBINDS  256
#define CONTROL 0x10000000
#define META    0x20000000
#define CTLX    0x40000000

enum keys {
  TAB_KEY = 9,
  ENTER_KEY = 13,
  BACKSPACE = 127,
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  PAGE_UP,
  PAGE_DOWN,
  HOME_KEY,
  END_KEY,
  DEL_KEY
};

typedef void (*key_func_t)(cursor_t *C);

typedef struct keytab {
  i32 code;
  key_func_t fp;
} keytab_t;

static keytab_t keytabs[NBINDS] = {
  { HOME_KEY, cursor_bol },
  { END_KEY, cursor_eol },
  { PAGE_DOWN, cursor_page_down },
  { PAGE_UP, cursor_page_up },
  { ARROW_LEFT, cursor_left },
  { ARROW_RIGHT, cursor_right },
  { ARROW_UP, cursor_up },
  { ARROW_DOWN, cursor_down },
  { BACKSPACE, cursor_remove_char },
  { DEL_KEY, cursor_remove_char },
  { ENTER_KEY, cursor_break_line },

  { CONTROL | 'H', cursor_remove_char },
  { CONTROL | 'A', cursor_bol },
  { CONTROL | 'E', cursor_eol },
  { CONTROL | 'K', cursor_delete_forward },
  { CONTROL | 'N', cursor_down },
  { CONTROL | 'P', cursor_up },
  { CONTROL | '/', cursor_undo },

  { META | 'f', cursor_move_word_forward },
  { META | 'b', cursor_move_word_backward },
  { META | '>', cursor_eof },
  { META | '<', cursor_bof },

  { 0, NULL}
};

static key_func_t get_kfp(int c) {
	keytab_t *ktp = &keytabs[0];

	while (ktp->fp != NULL) {
		if (ktp->code == c)
			return ktp->fp;
		++ktp;
	}

	return NULL;
}

static i32 input_read_key() {
  i32 nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
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

    return CONTROL | (c + '@');
  }

  return c;
}

static void process_insert_mode(cursor_t *C, i32 ch) {
  key_func_t kfp;
  if ((kfp = get_kfp(ch)) != NULL) {
    kfp(C);
    return;
  }

  if (ch == 'q'){
    if (C->editor->dirty)
      return mode_set_exit_save(C->editor);
    C->editor->running = false;
  } else if (ch == TAB_KEY) {
    cursor_insert_char(C, ' ');
    cursor_insert_char(C, ' ');
  } else if (ch >= 32 && ch <= 126) {
    cursor_insert_char(C, ch);
  }
}

void input_process_keys(cursor_t* C) {
  i32 ch = input_read_key();
  switch (g_mode) {
    case MODE_INSERT:
      process_insert_mode(C, ch);
      break;
    case MODE_CMD_CHAR:
      g_cmd_func(C->editor, ch);
      break;
  }
}
