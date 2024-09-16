#include "vt100.h"

#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 32

#define DELIMITER    ';'
#define ESC          '\x1b'
#define FINAL        'm'
#define OPEN_BRACKET '['

void tt_flush(void) {
  i32 status = fflush(stdout);
  if (status != 0 && errno != EAGAIN) exit(errno);
}

void tt_puts(char *buf) {
  fputs(buf, stdout);
}

static void tt_putc(char c) {
	fputc(c, stdout);
}

static void tt_puti(int n) {
  char buf[BUFSIZE];
  snprintf(buf, BUFSIZE, "%d", n);

  fputs(buf, stdout);
}

// Control Sequence Introducer
static void vt_csi(void) {
  tt_putc(ESC);
  tt_putc(OPEN_BRACKET);
}

void vt_erase_display(void) {
  tt_putc(ESC);
  tt_putc(OPEN_BRACKET);
  tt_putc('2');
  tt_putc('J');
}

void vt_set_cursor_position(i32 x, i32 y) {
  tt_putc(ESC);
  tt_putc(OPEN_BRACKET);
  tt_puti(x);
  tt_putc(DELIMITER);
  tt_puti(y);
  tt_putc('H');
}

void vt_clear_screen(void) {
  vt_erase_display();
  vt_set_cursor_position(0, 0);
}

void vt_cursor_forward(i32 pn) {
  if (pn <= 0) return;

  tt_putc(ESC);
  tt_putc(OPEN_BRACKET);
  tt_puti(pn);
  tt_putc('C');
}

void vt_cursor_down(i32 pn) {
  if (pn <= 0) return;

  tt_putc(ESC);
  tt_putc(OPEN_BRACKET);
  tt_puti(pn);
  tt_putc('B');
}

void vt_hide_cursor(void) {
  tt_puts("\x1b[?25l");
}

void vt_erase_line(void) {
  tt_puts("\x1b[K");
}

void vt_show_cursor(void) {
  tt_puts("\x1b[?25h");
}

i32 vt_cursor_position(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
    if (buf[i] == 'R') break;
    i++;
  }
  buf[i] = '\0';
  if (buf[0] != '\x1b' || buf[1] != '[') return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

  return 0;
}