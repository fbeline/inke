#include "terminal.h"

#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include "utils.h"

static struct termios oterm;

static void disable_raw_mode(void) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &oterm) == -1)
    die("tcsetattr");
}

static void enable_raw_mode(void) {
  if (tcgetattr(STDIN_FILENO, &oterm) == -1) die("tcgetattr");
  atexit(disable_raw_mode);
  struct termios raw = oterm;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}

static void draw(void) {
  int y;
  for (y = 0; y < 24; y++) {
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}

static void refresh_screen(void) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  draw();
}


void term_init(void) {
  enable_raw_mode();
}

void term_render(cursor_t *C) {
  refresh_screen();
}
