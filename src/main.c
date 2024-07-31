#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

#include "fs.h"
#include "editor.h"
#include "input.h"
#include "render.h"

static editor_t E = {0};

void Init(char* filename) {
  File file = FileRead(filename);
  E = editor_init(&file);

  render_init(1280, 720, 30);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("usage: olive [file path]\n");
    exit(1);
  }

  Init(argv[1]);

  while (!WindowShouldClose()) {
    input_mousewheel_handler(&E);
    input_keyboard_handler(&E);

    render_reload_font();

    render_draw(&E);
  }

  CloseWindow();        // Close window and OpenGL context

  return 0;
}
