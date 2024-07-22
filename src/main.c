#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vcruntime_string.h>
#include "fs.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define LINE_BREAK_SIZE 80

typedef struct text {
  char* renderData;
} Text;

static Text text = {};

void RenderData(File *file) {
  size_t renderDataSize = file->len + file->len/LINE_BREAK_SIZE + 1;
  text.renderData = malloc(renderDataSize);

  size_t counter = 0;
  for (size_t i = 0, j = 0; i < file->len; i++, j++) {
    counter++;
    if (counter <= LINE_BREAK_SIZE) {
      if (file->data[i] == '\n')
        counter = 0;

      text.renderData[j] = file->data[i];
    } else {
      counter = 0;
      j++;
      text.renderData[j] = '\n';
    }
  }
}

int main(void) {
  File file = FileRead("notes.txt");
  RenderData(&file);

  int screenWidth = 768;
  int screenHeight = 432;

  InitWindow(screenWidth, screenHeight, "Olive");
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
  Font customFont = LoadFont("resources/cmunsl.ttf"); 
  GenTextureMipmaps(&customFont.texture); 
  SetTextureFilter(customFont.texture, TEXTURE_FILTER_BILINEAR);
  int baseFontSize = customFont.baseSize;

  SetTargetFPS(60);

  while (!WindowShouldClose()) {

    // RELOAD FONT IF SCREEN SIZE CHANGES
    if (GetScreenWidth() != screenWidth || GetScreenHeight() != screenHeight) {
      float scale = MIN((float)GetScreenWidth()/screenWidth, (float)GetScreenHeight()/screenHeight);
      baseFontSize *= scale;

      // Unload previous font
      UnloadFont(customFont);

      // Load new font with updated size
      customFont = LoadFontEx("resources/cmunsl.ttf", baseFontSize, NULL, 250);
      GenTextureMipmaps(&customFont.texture); 
      SetTextureFilter(customFont.texture, TEXTURE_FILTER_BILINEAR);

      // Update previous window size
      screenWidth = GetScreenWidth();
      screenHeight = GetScreenHeight();
    }

    float lineSpacing = customFont.baseSize * 1.5f;



    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawTextEx(customFont, text.renderData, (Vector2){ 10, 10 }, baseFontSize, 0, DARKGRAY);

    EndDrawing();
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow();        // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
