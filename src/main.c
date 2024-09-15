#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>

#include "editor.h"
#include "cursor.h"
#include "input.h"
#include "utils.h"

static editor_t E = {0};
static cursor_t C = {0};
struct termios orig_termios;

void Init(const char* filename) {
  E = editor_init(filename);
  C = cursor_init(&E);
}

static void disable_raw_mode(void) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

static void enable_raw_mode(void) {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
  atexit(disable_raw_mode);
  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}

void draw(void) {
  int y;
  for (y = 0; y < 24; y++) {
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}

void refresh_screen(void) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  draw();
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("usage: inke [file path]\n");
    exit(1);
  }

  Init(argv[1]);
  enable_raw_mode();

  for(;;) {
    refresh_screen();
    input_process_keys(&C);
  }

  return 0;
}
