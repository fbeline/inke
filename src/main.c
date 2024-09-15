#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "editor.h"
#include "cursor.h"
#include "input.h"
#include "terminal.h"
#include "utils.h"

static editor_t E = {0};
static cursor_t C = {0};

void Init(const char* filename) {
  E = editor_init(filename);
  C = cursor_init(&E);
  term_init();
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("usage: inke [file path]\n");
    exit(1);
  }

  Init(argv[1]);

  for(;;) {
    term_render(&C);
    input_process_keys(&C);
  }

  return 0;
}
