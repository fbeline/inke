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

void vt_clear_screen(void) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
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
  tt_putc('C');
}
