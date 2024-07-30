#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

#include "fs.h"
#include "editor.h"
#include "input.h"
#include "render.h"

static Editor E = {0};

void Init(char* filename) {
  File file = FileRead(filename);
  E = editor_init(&file);

  // WINDOW INIT
  E.window = (Window){1280, 720};
  InitWindow(E.window.width, E.window.height, "Olive");
  SetWindowState(FLAG_WINDOW_RESIZABLE);

  SetTargetFPS(60);

  // FONT INIT
  E.font.size = 30;
  render_load_font(&E);
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

    // RELOAD FONT IF SCREEN SIZE CHANGES
    if (GetScreenWidth() != E.window.width || GetScreenHeight() != E.window.height) {
      render_reload_font(&E); 
    }

    render_draw(&E);
  }

  // == BEGIN De-Initialization ===
  CloseWindow();        // Close window and OpenGL context
  // === END DE-INITIALIZATION ===

  return 0;
}
