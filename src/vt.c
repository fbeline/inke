#include "vt.h"

#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>

#include "ds.h"
#include "utils.h"

#define PUTSIZE 1024

static ds_t *sbuffer;

void vt_flush(void) {
  if (write(STDOUT_FILENO, sbuffer->buf, sbuffer->len) != sbuffer->len)
    die("stdout write error");
  sbuffer->buf[0] = '\0';
  sbuffer->len = 0;
}

void vt_puts(char *str, ...) {
  char fmtstr[PUTSIZE];
  va_list args;
	va_start(args, str);
  vsnprintf(fmtstr, PUTSIZE, str, args);
	va_end(args);
  dscat(sbuffer, fmtstr);
}

void vt_nputs(char *str, usize n) {
  if (n <= 0) return;
  dsncat(sbuffer, str, n);
}

void vt_erase_display(void) {
  vt_puts("\x1b[2J");
}

void vt_set_cursor_position(i32 x, i32 y) {
  vt_puts("\x1b[%d;%dH", x, y);
}

void vt_clear_screen(void) {
  vt_erase_display();
  vt_set_cursor_position(0, 0);
}

void vt_cursor_forward(i32 pn) {
  if (pn <= 0) return;

  vt_puts("\x1b[%dC", pn);
}

void vt_cursor_down(i32 pn) {
  if (pn <= 0) return;

  vt_puts("\x1b[%dB", pn);
}

void vt_hide_cursor(void) {
  vt_puts("\x1b[?25l");
}

void vt_erase_line(void) {
  vt_puts("\x1b[K");
}

void vt_show_cursor(void) {
  vt_puts("\x1b[?25h");
}

void vt_reset_text_attr(void) {
  vt_puts("\x1b[0m");
}

void vt_reverse_video(void) {
  vt_puts("\x1b[7m");
}

void vt_init(void) {
  sbuffer = dsempty();
  vt_clear_screen();
}
