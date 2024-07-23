#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vcruntime_string.h>
#include "fs.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MAX_LINES 25
#define LINE_BREAK_SIZE 75
#define DUMMY_LINE "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

typedef struct editorFont {
  Font font;
  unsigned short size;
  unsigned short lineSpacing;
} EditorFont;

typedef struct window {
  int width;
  int height;
} Window;

typedef struct config {
  int cx, cy;
  int rowoff;
  int screenrows;
  int screencols;
  Vector2 renderPos;
  unsigned int numrows;
  EditorFont font;
  char** buffer;
  Window window;
} Config;

static Config C = {0};

char Next(File* file, size_t i) {
  if (i + 1 >= file->len)
    return '\0';

  return file->data[i + 1];
}

void RenderData(File *file) {
  size_t lsize = 10;
  C.buffer = (char**)malloc(lsize * sizeof(char*));

  unsigned int numrows = 0;
  size_t lineStart = 0;
  size_t lineSize = 0;
  for (size_t i = 0; i < file->len; i++, lineSize++) {
    if (file->data[i] == '\n' || file->data[i] == '\r' || lineSize >= LINE_BREAK_SIZE) {
      C.buffer[numrows] = (char*)malloc(lineSize + 1);
      for (size_t k = 0; lineStart + k < i; k++) {
        size_t fileIdx = lineStart + k;
        char c = file->data[fileIdx];
        if (c == '\r' && Next(file, fileIdx) == '\n') i++; // jump to next line when \r\n
        if (c == '\n' || c == '\r') c = '\0';
        if (c == '\t') c = ' ';
        C.buffer[numrows][k] = c;
      }
      C.buffer[numrows][lineSize] = '\0';

      numrows++;
      lineStart = i;
      lineSize = 0;
      if (numrows >= lsize) {
        lsize *= 2;
        char **tmp = realloc(C.buffer, lsize * sizeof(char*));
        C.buffer = tmp;
      }
    }
  }
  C.numrows = numrows;
}

void LoadCustomFont(void) {
  printf("FONT SIZE %d\n", C.font.size);
  Font customFont = LoadFontEx("resources/cmunsl.ttf", C.font.size, NULL, 250); 
  C.font.lineSpacing = C.font.size * 1.15;
  SetTextLineSpacing(C.font.lineSpacing);
  GenTextureMipmaps(&customFont.texture); 
  SetTextureFilter(customFont.texture, TEXTURE_FILTER_BILINEAR);
  C.font.font = customFont;

  Vector2 lineSize = 
    MeasureTextEx(customFont,
                  DUMMY_LINE,
                  C.font.size,
                  0);
  C.renderPos.x = (C.window.width - lineSize.x) / 2;
  C.renderPos.y = 10;
}

void Init(void) {
  // DATA INIT
  File file = FileRead("notes.txt");
  RenderData(&file);

  // WINDOW INIT
  C.window = (Window){1280, 720};
  InitWindow(C.window.width, C.window.height, "Olive");
  SetWindowState(FLAG_WINDOW_RESIZABLE);

  SetTargetFPS(60);

  // FONT INIT
  C.font.size = 30;
  LoadCustomFont();
}

void MouseWheelHandler(void) {
  float mouseWheelMove = GetMouseWheelMove();
  if (mouseWheelMove > 0) {
    C.rowoff = MAX(C.rowoff - 1, 0);
  }
  else if (mouseWheelMove < 0) {
    C.rowoff = MIN(C.rowoff + 1, C.numrows - 1);  // Mouse wheel down
  }
}

int main(void) {
  Init();

  while (!WindowShouldClose()) {
    MouseWheelHandler();

    // RELOAD FONT IF SCREEN SIZE CHANGES
    if (GetScreenWidth() != C.window.width || GetScreenHeight() != C.window.height) {
      float scale = (float)GetScreenWidth() / C.window.width;
      C.window = (Window) { GetScreenWidth(), GetScreenHeight() };

      UnloadFont(C.font.font);

      C.font.size *= scale;
      LoadCustomFont();
    }

    // === BEGIN DRAWING ===
    BeginDrawing();

    ClearBackground(RAYWHITE);

    for (size_t i = 0; i < MAX_LINES && i + C.rowoff < C.numrows; i++) {
      DrawTextEx(C.font.font,
                 C.buffer[i + C.rowoff],
                 (Vector2){C.renderPos.x, C.font.lineSpacing * i + 10},
                 C.font.size,
                 0,
                 DARKGRAY);
    }

    EndDrawing();
    // === END DRAWING ===
  }

  // == BEGIN De-Initialization ===
  CloseWindow();        // Close window and OpenGL context
  // === END DE-INITIALIZATION ===

  return 0;
}
