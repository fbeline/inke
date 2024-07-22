#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vcruntime_string.h>
#include "fs.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MAX_LINES 25
#define LINE_BREAK_SIZE 66
#define DUMMY_LINE "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

typedef struct editorFont {
  Font font;
  unsigned short size;
  unsigned short lineSpacing;
} EditorFont;

typedef struct window {
  int width;
  int height;
} Window;

typedef struct editorConfig {
  int cx, cy;
  int rowoff;
  int screenrows;
  int screencols;
  Vector2 renderPos;
  unsigned int numrows;
  EditorFont font;
  char** renderData;
} EditorConfig;

static EditorConfig text = {0};
static Window window = {0};

void RenderData(File *file) {
  size_t renderDataSize = file->len + file->len/LINE_BREAK_SIZE + 1;
  text.renderData = malloc(renderDataSize);

  unsigned int numrows = 0;
  size_t lineStart = 0;
  size_t lineSize = 0;
  for (size_t i = 0; i < file->len; i++, lineSize++) {
    if (file->data[i] == '\n' || lineSize >= LINE_BREAK_SIZE) {
      text.renderData[numrows] = malloc(lineSize + 1);
      for (size_t k = 0; lineStart + k < i; k++) {
        char c = file->data[lineStart + k];
        if (c == '\n' || c == '\r') c = '\0';
        if (c == '\t') c = ' ';
        text.renderData[numrows][k] = c;
        if (numrows == 6)
          printf("LINE SIZE: %zu, K: %zu\n", lineSize, k);
      }
      text.renderData[numrows][lineSize] = '\0';

      numrows++;
      lineStart = i;
      lineSize = 0;
    }
  }
  text.numrows = numrows;
}

void LoadCustomFont(void) {
  printf("FONT SIZE %d\n", text.font.size);
  Font customFont = LoadFontEx("resources/cmunsl.ttf", text.font.size, NULL, 250); 
  text.font.lineSpacing = text.font.size * 1.15;
  SetTextLineSpacing(text.font.lineSpacing);
  GenTextureMipmaps(&customFont.texture); 
  SetTextureFilter(customFont.texture, TEXTURE_FILTER_BILINEAR);
  text.font.font = customFont;

  Vector2 lineSize = 
    MeasureTextEx(customFont,
                  DUMMY_LINE,
                  text.font.size,
                  0);
  text.renderPos.x = (window.width - lineSize.x) / 2;
  text.renderPos.y = 10;
}

void InitEditor(void) {
  // DATA INIT
  File file = FileRead("notes.txt");
  RenderData(&file);

  // WINDOW INIT
  window.width = 1920;
  window.height = 1080;

  InitWindow(window.width, window.height, "Olive");
  SetWindowState(FLAG_WINDOW_RESIZABLE);

  SetTargetFPS(60);

  // FONT INIT
  text.font.size = 35;
  LoadCustomFont();

  /* printf(">> %s\n", text.renderData[33]); */
}

void MouseWheelHandler(void) {
  float mouseWheelMove = GetMouseWheelMove();
  if (mouseWheelMove > 0) {
    text.rowoff = MAX(text.rowoff - 1, 0);
  }
  else if (mouseWheelMove < 0) {
    text.rowoff = MIN(text.rowoff + 1, text.numrows - 1);  // Mouse wheel down
  }
}

int main(void) {
  InitEditor();

  while (!WindowShouldClose()) {
    MouseWheelHandler();

    // RELOAD FONT IF SCREEN SIZE CHANGES
    if (GetScreenWidth() != window.width || GetScreenHeight() != window.height) {
      float scale = (float)GetScreenWidth() / window.width;
      window.width = GetScreenWidth();
      window.height = GetScreenHeight();

      UnloadFont(text.font.font);

      text.font.size *= scale;
      LoadCustomFont();
    }

    // === BEGIN DRAWING ===
    BeginDrawing();

    ClearBackground(RAYWHITE);
    for (size_t i = 0; i < MAX_LINES && i + text.rowoff <= text.numrows; i++) {
      /* printf("DRAW LINE I: %zu, ROWOFF: %u\n", i, text.rowoff); */
      DrawTextEx(text.font.font,
                 text.renderData[i + text.rowoff],
                 (Vector2){text.renderPos.x, text.font.lineSpacing * i + 10},
                 text.font.size,
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
