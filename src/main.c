#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "fs.h"
#include "editor.h"
#include "input.h"
#include "render.h"

void LoadCustomFont(void) {
  Font customFont = LoadFontEx("resources/FiraCode-Regular.ttf", E.font.size, NULL, 250); 
  E.font.lineSpacing = E.font.size * 1.15;
  SetTextLineSpacing(E.font.lineSpacing);
  GenTextureMipmaps(&customFont.texture); 
  SetTextureFilter(customFont.texture, TEXTURE_FILTER_BILINEAR);
  E.font.font = customFont;

  Vector2 lineSize = 
    MeasureTextEx(customFont,
                  DUMMY_LINE,
                  E.font.size,
                  0);

  E.eMargin = (Vector2) {
    (E.window.width - lineSize.x) / 2,
    E.window.height * 0.07
  };
}

void Init(char* filename) {
  // DATA INIT
  memcpy(E.filename, filename, strlen(filename));
  E.filename[strlen(filename)] = '\0';
  File file = FileRead(filename);
  BuildRows(&file);

  // WINDOW INIT
  E.window = (Window){1280, 720};
  InitWindow(E.window.width, E.window.height, "Olive");
  SetWindowState(FLAG_WINDOW_RESIZABLE);

  SetTargetFPS(60);

  // FONT INIT
  E.font.size = 30;
  LoadCustomFont();
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("usage: olive [file path]\n");
    exit(1);
  }

  Init(argv[1]);

  while (!WindowShouldClose()) {
    MouseWheelHandler();
    KeyboardHandler();

    // RELOAD FONT IF SCREEN SIZE CHANGES
    if (GetScreenWidth() != E.window.width || GetScreenHeight() != E.window.height) {
      float scale = (float)GetScreenWidth() / E.window.width;
      E.window = (Window) { GetScreenWidth(), GetScreenHeight() };

      UnloadFont(E.font.font);

      E.font.size *= scale;
      LoadCustomFont();
    }

    render_draw();
  }

  // == BEGIN De-Initialization ===
  CloseWindow();        // Close window and OpenGL context
  // === END DE-INITIALIZATION ===

  return 0;
}
