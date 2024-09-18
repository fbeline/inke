#include "terminal.h"

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "mode.h"
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

static void term_draw_status_bar(term_t *T, cursor_t *C, line_t *buffer) {
  line_append(buffer, "\x1b[K\x1b[7m");

  char *status;
  if ((status = malloc(T->cols + 1)) == NULL) die("out of memory");

  i32 len = snprintf(status,
                     T->cols + 1, "%.20s %s%*s%d,%d",
                     C->editor->filename,
                     C->editor->dirty ? "[+]" : "",
                     T->cols - 20,
                     "",
                     C->x + 1,
                     C->y + 1
                     );

  line_append(buffer, status);

  for (i32 i = len; i < T->cols; i++)
    line_append(buffer, " ");

  line_append(buffer, "\r\n\x1b[K\x1b[m");
  line_append(buffer, mode_get_message());

  free(status);
}

static void term_draw(term_t *T, cursor_t *C) {
  int y;
  line_t *lp = C->editor->lines;
  line_t *buffer = lalloc(32);

  for (i32 i = 0; i < C->rowoff && lp->next != NULL; i++) {
    lp = lp->next;
  }

  for (y = 0; y < T->rows; y++) {
    line_append(buffer, "\x1b[K");

    if (lp != NULL) {
      i32 size = (i32)lp->size - C->coloff;
      size = CLAMP(size, 0, T->cols);
      line_append_s(buffer, lp->text + C->coloff, size);
      lp = lp->next;
    }

    line_append(buffer, "\r\n");
  }

  term_draw_status_bar(T, C, buffer);

  write(STDOUT_FILENO, buffer->text, buffer->size);

  line_free(buffer);
}

static i32 term_get_size(term_t *T) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    return -1;
  } else {
    T->cols = ws.ws_col;
    T->rows = ws.ws_row - 2;
    return 0;
  }
}

void term_init(void) {
  enable_raw_mode(&T);
  vt_erase_display();
  if (term_get_size(&T) == -1) die("term_get_size");
}

void term_restore(void) {
  vt_erase_display();
  disable_raw_mode();
}

void term_render(cursor_t *C) {
  cursor_set_max(C, T.cols, T.rows - 1);
  vt_set_cursor_position(0, 0);
  vt_hide_cursor();
  tt_flush();

  term_draw(&T, C);

  vt_set_cursor_position(C->y + 1, C->x + 1);
  vt_show_cursor();
  tt_flush();
}
