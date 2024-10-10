#include "terminal.h"

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#include "cmdline.h"
#include "globals.h"
#include "utils.h"
#include "vt.h"

static term_t T;

void term_restore(void) {
  vt_erase_display();
  vt_show_cursor();
  vt_set_cursor_position(0, 0);
  vt_flush();
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &T.oterm) == -1)
    DIE("tcsetattr");
}

void signal_handler(int signum) {
  term_restore();
  DIE("Caught signal %d, exiting...", signum);
}

static void enable_raw_mode(term_t *T) {
  if (tcgetattr(STDIN_FILENO, &T->oterm) == -1) DIE("tcgetattr");

  atexit(term_restore);
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

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) DIE("tcsetattr");
}

static void term_draw_status_bar(term_t *T, buffer_t *B) {
  vt_erase_line();
  vt_reverse_video();

  char *status;
  if ((status = malloc(T->cols + 1)) == NULL) DIE("out of memory");

  i32 len = snprintf(status,
                     T->cols + 1, "%.20s %s%*s%zu,%zu",
                     B->name,
                     B->dirty > 0 ? "[+]" : "",
                     (i32)T->cols - 20,
                     "",
                     B->cursor->x + B->cursor->coloff + 1lu,
                     B->cursor->y + B->cursor->rowoff + 1lu
                     );

  vt_puts(status);

  for (usize i = len; i < T->cols; i++)
    vt_puts(" ");

  vt_puts("\r\n");
  vt_reset_text_attr();
  vt_erase_line();

  if (g_mode & (MODE_CMD | MODE_SEARCH))
    vt_puts(cmdline()->ds->buf);
  else
    vt_puts(get_status_message());

  free(status);
}

static void term_draw_mark(term_t *T, cursor_t *C, line_t *lp, char *line) {
  mark_t mark = mark_get();
  mark.start_offset = mark.start_offset > C->coloff ? mark.start_offset - C->coloff : 0;
  mark.end_offset = mark.end_offset > C->coloff ? mark.end_offset - C->coloff : 0;

  if (lp == mark.start_lp && lp == mark.end_lp) {
    vt_nputs(line, mark.start_offset);
    vt_reverse_video();
    vt_nputs(line + mark.start_offset, mark.end_offset - mark.start_offset);
    vt_reset_text_attr();
    vt_puts(line + mark.end_offset);
  } else if (lp == mark.start_lp) {
    vt_nputs(line, mark.start_offset);
    vt_reverse_video();
    vt_puts(line + mark.start_offset);
  } else if (lp == mark.end_lp) {
    vt_nputs(line, mark.end_offset);
    vt_reset_text_attr();
    vt_puts(line + mark.end_offset);
  } else {
    vt_puts(line);
  }
}

static void term_draw_line(term_t *T, cursor_t *C, line_t *lp) {
  if (lp == NULL) return;

  char line[1024];
  usize size = lp->ds->len > C->coloff ? lp->ds->len - C->coloff : 0;
  size = CLAMP(size, 0, T->cols);
  strncpy(line, lp->ds->buf + C->coloff, size);
  line[size] = '\0';

  if ((g_mode & MODE_SEARCH) && lp == g_isearch.lp && g_isearch.x >= C->coloff) {
    u32 sx = g_isearch.x - C->coloff;
    vt_nputs(line, sx);
    vt_reverse_video();
    vt_nputs(line + sx, g_isearch.qlen);
    vt_reset_text_attr();
    vt_puts(line + sx + g_isearch.qlen);
  } else if (g_mode & MODE_VISUAL) {
    term_draw_mark(T, C, lp, line);
  } else {
    vt_puts(line);
  }
}

static void term_draw(term_t *T, buffer_t *B) {
  usize y;
  line_t *lp = B->editor->lines;
  cursor_t *C = B->cursor;

  for (usize i = 0; i < C->rowoff && lp->next != NULL; i++) {
    if (g_mode == MODE_VISUAL && g_mark.start_lp == lp)
      vt_reverse_video();

    lp = lp->next;
  }

  for (y = 0; y < T->rows; y++) {
    vt_erase_line();
    term_draw_line(T, C, lp);
    vt_puts("\r\n");

    if (lp != NULL) lp = lp->next;
  }

  term_draw_status_bar(T, B);
}

static i32 __term_update_size(term_t *T) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    return -1;
  } else {
    T->cols = ws.ws_col;
    T->rows = ws.ws_row - 2;
    return 0;
  }
}

void term_update_size(void) {
  if (__term_update_size(&T) == -1)
    DIE("Unable to get screen size");
}

void term_get_size(u16 *rows, u16 *cols) {
  *rows = T.rows;
  *cols = T.cols;
}

void term_init(void) {
  enable_raw_mode(&T);
  vt_init();
  term_update_size();
}

void term_render(buffer_t *B) {
  g_window.ncol = T.cols;
  g_window.nrow = T.rows - 1;
  vt_set_cursor_position(0, 0);
  vt_hide_cursor();

  term_draw(&T, B);

  if (g_mode & (MODE_CMD | MODE_SEARCH))
    vt_set_cursor_position(T.rows + 2, cmdline()->x + 1);
  else
    vt_set_cursor_position(B->cursor->y + 1, B->cursor->x + 1);

  if (g_cursor_vis) vt_show_cursor();

  vt_flush();
}
