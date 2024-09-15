#include "input.h"
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "utils.h"
#include "vt100.h"

#define CTRL_KEY(k) ((k) & 0x1f)

static char input_read_key() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}

void input_process_keys(cursor_t* C) {
  char c = input_read_key();
  switch (c) {
    case CTRL_KEY('q'):
      vt_clear_screen();
      exit(0);
      break;
  }
}
