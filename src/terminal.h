#pragma once

#include <termios.h>
#include "cursor.h"

typedef struct term_s {
  struct termios oterm;
  i32 cols, rows;
} term_t;

void term_init(void);

void term_restore(void);

void term_render(cursor_t *C);
