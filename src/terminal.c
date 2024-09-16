#include "terminal.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "utils.h"
#include "vt100.h"

static term_t T;

static void disable_raw_mode(void) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &T.oterm) == -1)
    die("tcsetattr");
}

static void enable_raw_mode(term_t *T) {
  if (tcgetattr(STDIN_FILENO, &T->oterm) == -1) die("tcgetattr");
  atexit(disable_raw_mode);
  struct termios raw = T->oterm;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}

static void term_draw(term_t *T, cursor_t *C) {
  int y;
  line_t *lp = C->editor->lines;
  line_t *buffer = lalloc(32);

  for (y = 0; y < T->rows && lp != NULL; y++) {
    line_append(buffer, lp->text);
    if (y < T->rows - 1) {
      line_append(buffer, "\r\n");
    }
    lp = lp->next;
  }

  tt_puts(buffer->text);

  line_free(buffer);
}

static i32 term_get_size(term_t *T) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    return -1;
  } else {
    T->cols = ws.ws_col;
    T->rows = ws.ws_row;
    return 0;
  }
}

void term_init(void) {
  enable_raw_mode(&T);
  if (term_get_size(&T) == -1) die("term_get_size");
}

void term_render(cursor_t *C) {
  vt_erase_display();
  vt_set_cursor_position(0, 0);
  vt_hide_cursor();
  tt_flush();

  term_draw(&T, C);

  vt_show_cursor();
  vt_set_cursor_position(C->x, C->y);
  tt_flush();
}
