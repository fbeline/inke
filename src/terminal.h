#pragma once

#include <termios.h>
#include "types.h"

typedef struct term_s {
  struct termios oterm;
  usize cols, rows;
} term_t;

void term_init(void);

void term_render(cursor_t *C);

void term_restore(void);

void term_get_size(u16 *rows, u16 *cols);

void term_update_size(void);
