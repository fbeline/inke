#include "terminal.h"

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "utils.h"
#include "vt100.h"

static term_t T;

void disable_raw_mode(void) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &T.oterm) == -1)
    die("tcsetattr");
}

void signal_handler(int signum) {
  disable_raw_mode();
  die("Caught signal %d, exiting...", signum);
}

static void enable_raw_mode(term_t *T) {
  if (tcgetattr(STDIN_FILENO, &T->oterm) == -1) die("tcgetattr");

  atexit(disable_raw_mode);
  signal(SIGTERM, signal_handler);
  signal(SIGSEGV, signal_handler);
  signal(SIGABRT, signal_handler);

  struct termios raw = T->oterm;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

static void term_draw(term_t *T, cursor_t *C) {
  int y;
  line_t *lp = C->editor->lines;
  line_t *buffer = lalloc(32);

  for (i32 i = 0; i < C->rowoff && lp->next != NULL; i++) {
    lp = lp->next;
  }

  for (y = 0; y < T->rows && lp != NULL; y++) {
    i32 size = (i32)lp->size - C->coloff;
    size = CLAMP(size, 0, T->cols);
    line_append_s(buffer, lp->text + C->coloff, size);
    lp = lp->next;

    if (y < T->rows - 1) line_append(buffer, "\r\n");
  }

  write(STDOUT_FILENO, buffer->text, buffer->size);

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
  vt_erase_display();
  if (term_get_size(&T) == -1) die("term_get_size");
}

void term_render(cursor_t *C) {
  C->max_col = T.cols;
  C->max_row = T.rows;

  vt_erase_display();
  vt_set_cursor_position(0, 0);
  vt_hide_cursor();
  tt_flush();

  term_draw(&T, C);

  vec2_t pos = cursor_position(C);
  vt_set_cursor_position(pos.y + 1, pos.x + 1);
  vt_show_cursor();
  tt_flush();
}
