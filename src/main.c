#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "editor.h"
#include "input.h"
#include "render.h"

static editor_t E = {0};
static render_state_t R = {0};

void Init(const char* filename) {
  E = editor_init(filename);
  R = render_init(1280, 720, 30);
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

    render_reload_font(&R);
    render_draw(&E, &R);
  }

  CloseWindow();        // Close window and OpenGL context

  return 0;
}
