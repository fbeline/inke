#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "buffer.h"
#include "globals.h"
#include "editor.h"
#include "cursor.h"
#include "input.h"
#include "terminal.h"

static void handle_sigwinch(int unused __attribute__((unused))) {
  u16 rows, cols;
  buffer_t *buffer = buffer_get();
  term_update_size();
  term_get_size(&rows, &cols);
  cursor_update_window_size(buffer->cursor, rows - 1, cols);
  term_render(buffer);
}

static void init(const char* filename) {
  buffer_create(filename);
  globals_init();
  term_init();

  signal(SIGWINCH, handle_sigwinch);
}

int main(int argc, char **argv) {
  if (argc < 2 || strlen(argv[1]) == 0) {
    printf("usage: inke [file path]\n");
    exit(1);
  }

  init(argv[1]);

  for(;;) {
    if (!g_running) break;

    buffer_t *buffer = buffer_get();
    term_render(buffer);
    input_process_keys(buffer);
  }

  term_restore();

  return 0;
}
