#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vcruntime_string.h>
#include "fs.h"

#define LINE_BREAK_SIZE 80

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
  unsigned int numrows;
  EditorFont font;
  char* renderData;
} EditorConfig;

static EditorConfig text = {0};
static Window window = {0};

void RenderData(File *file) {
  size_t renderDataSize = file->len + file->len/LINE_BREAK_SIZE + 1;
  text.renderData = malloc(renderDataSize);

  size_t counter = 0;
  unsigned int numrows = 0;
  for (size_t i = 0, j = 0; i < file->len; i++, j++) {
    counter++;
    if (counter <= LINE_BREAK_SIZE) {
      if (file->data[i] == '\n') {
        counter = 0;
        numrows++;
      }

      text.renderData[j] = file->data[i];
    } else {
      counter = 0;
      j++; numrows++;
      text.renderData[j] = '\n';
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
}

int main(void) {
  InitEditor();

  while (!WindowShouldClose()) {

    // RELOAD FONT IF SCREEN SIZE CHANGES
    if (GetScreenWidth() != window.width || GetScreenHeight() != window.height) {
      float scale = (float)GetScreenWidth() / window.width;
      text.font.size *= scale;

      UnloadFont(text.font.font);
      LoadCustomFont();

      window.width = GetScreenWidth();
      window.height = GetScreenHeight();
    }


    // === BEGIN DRAWING ===
    BeginDrawing();

    ClearBackground(RAYWHITE);
    DrawTextEx(text.font.font, text.renderData, (Vector2){ 10, 10 }, text.font.size, 0, DARKGRAY);

    EndDrawing();
    // === END DRAWING ===
  }

  // == BEGIN De-Initialization ===
  CloseWindow();        // Close window and OpenGL context
  // === END DE-INITIALIZATION ===

  return 0;
}
