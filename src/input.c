#include "input.h"

#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "utils.h"
#include "vt100.h"

#define CTRL_KEY(k) ((k) & 0x1f)

enum keys {
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

static i32 input_read_key() {
  i32 nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }

  if (c == '\x1b') {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
    if (seq[0] == '[') {
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
      switch (seq[1]) {
        case 'H': return HOME_KEY;
        case 'F': return END_KEY;
      }

      return '\x1b';
    }
  }

  return c;
}

void input_process_keys(cursor_t* C) {
  i32 c = input_read_key();
  switch (c) {
    case CTRL_KEY('p'):
    case ARROW_UP:
      cursor_up(C);
      break;
    case CTRL_KEY('n'):
    case ARROW_DOWN:
      cursor_down(C);
      break;
    case CTRL_KEY('f'):
    case ARROW_RIGHT:
      cursor_right(C);
      break;
    case CTRL_KEY('b'):
    case ARROW_LEFT:
      cursor_left(C);
      break;
    case PAGE_UP:
      cursor_page_up(C);
      break;
    case PAGE_DOWN:
      cursor_page_down(C);
      break;
    case CTRL_KEY('a'):
    case HOME_KEY:
      cursor_bol(C);
      break;
    case CTRL_KEY('e'):
    case END_KEY:
      cursor_eol(C);
      break;
    case CTRL_KEY('q'):
      vt_clear_screen();
      exit(0);
      break;
  }
}
