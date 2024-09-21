#include "terminal.h"

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#include "ds.h"
#include "globals.h"
#include "mode.h"
#include "utils.h"
#include "vt.h"

static term_t T;

void term_restore(void) {
  vt_erase_display();
  vt_show_cursor();
  vt_set_cursor_position(0, 0);
  vt_flush();
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &T.oterm) == -1)
    die("tcsetattr");
}

void signal_handler(int signum) {
  term_restore();
  die("Caught signal %d, exiting...", signum);
}

static void enable_raw_mode(term_t *T) {
  if (tcgetattr(STDIN_FILENO, &T->oterm) == -1) die("tcgetattr");

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

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

static void term_draw_status_bar(term_t *T, cursor_t *C) {
  vt_erase_line();
  vt_reverse_video();

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

  vt_puts(status);

  for (i32 i = len; i < T->cols; i++)
    vt_puts(" ");

  vt_puts("\r\n");
  vt_reset_text_attr();
  vt_erase_line();
  vt_puts(get_status_message());

  free(status);
}

static void term_draw_line(term_t *T, cursor_t *C, line_t *lp) {
  if (lp == NULL) return;

  char line[1024];
  i32 size = (i32)lp->size - C->coloff;
  size = CLAMP(size, 0, T->cols);
  strncpy(line, lp->text + C->coloff, size);
  line[size] = '\0';

  if (g_mode != MODE_VISUAL)
    return vt_puts(line);

  i32 region_offset = C->region.offset - C->coloff;
  if (lp == C->region.lp && lp == C->clp){ // region same line
    vt_nputs(line, region_offset);
    vt_reverse_video();
    vt_nputs(line + region_offset, C->region.size);
    vt_reset_text_attr();
    vt_puts(line + region_offset + C->region.size);
  } else if (lp == C->region.lp) { // region start
    vt_nputs(line, region_offset);
    vt_reverse_video();
    vt_puts(line + region_offset);
  } else if (lp == C->clp) { // region end
    vt_nputs(line, C->x);
    vt_reset_text_attr();
    vt_puts(line + C->x);
  } else {
    vt_puts(line);
  }
}

static void term_draw(term_t *T, cursor_t *C) {
  int y;
  line_t *lp = C->editor->lines;

  for (i32 i = 0; i < C->rowoff && lp->next != NULL; i++) {
    if (g_mode == MODE_VISUAL && C->region.lp == lp)
      vt_reverse_video();

    lp = lp->next;
  }

  for (y = 0; y < T->rows; y++) {
    vt_erase_line();
    term_draw_line(T, C, lp);
    vt_puts("\r\n");

    if (lp != NULL) lp = lp->next;
  }

  term_draw_status_bar(T, C);
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
  vt_init();
  if (term_get_size(&T) == -1) die("term_get_size");
}

void term_render(cursor_t *C) {
  cursor_set_max(C, T.cols, T.rows - 1);
  vt_set_cursor_position(0, 0);
  vt_hide_cursor();

  term_draw(&T, C);

  vt_set_cursor_position(C->y + 1, C->x + 1);
  if (g_cursor_vis) vt_show_cursor();

  vt_flush();
}
