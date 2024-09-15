#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "editor.h"
#include "cursor.h"

static editor_t E = {0};
static cursor_t C = {0};

void Init(const char* filename) {
  E = editor_init(filename);
  C = cursor_init(&E);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("usage: olive [file path]\n");
    exit(1);
  }

  Init(argv[1]);

  return 0;
}
