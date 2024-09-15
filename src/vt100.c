#include "vt100.h"

#include <termios.h>
#include <unistd.h>

void vt_clear_screen(void) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
}
