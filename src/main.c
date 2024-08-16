#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "editor.h"
#include "input.h"
#include "render.h"

static editor_t E = {0};

void Init(const char* filename) {
  E = editor_init(filename);

  render_init(1280, 720, 30);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("usage: olive [file path]\n");
    exit(1);
  }

  Init(argv[1]);
  bool running = true;
  while (!WindowShouldClose() && running) {
    input_keyboard_handler(&E, &running);

    render_reload_font();
    render_draw(&E);
  }

  CloseWindow();        // Close window and OpenGL context

  return 0;
}
