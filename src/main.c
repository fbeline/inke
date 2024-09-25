#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "globals.h"
#include "editor.h"
#include "cursor.h"
#include "input.h"
#include "terminal.h"

static editor_t E = {0};
static cursor_t C = {0};

static void handle_sigwinch(int unused __attribute__((unused))) {
  u16 rows, cols;
  term_update_size();
  term_get_size(&rows, &cols);
  cursor_update_window_size(&C, rows - 1, cols);
  term_render(&C);
}

static void init(const char* filename) {
  E = editor_init(filename);
  C = cursor_init(&E);
  globals_init();
  term_init();

  signal(SIGWINCH, handle_sigwinch);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("usage: inke [file path]\n");
    exit(1);
  }

  init(argv[1]);

  for(;;) {
    if (!g_running) break;

    term_render(&C);
    input_process_keys(&C);
  }

  term_restore();

  return 0;
}
