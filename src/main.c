#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vcruntime_string.h>
#include "fs.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

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

typedef struct config {
  int cx, cy;
  int rowoff;
  int screenrows;
  int screencols;
  Vector2 eMargin;
  EditorFont font;
  unsigned int rowslen;
  char** rows;
  Window window;
} Config;

static Config C = {0};

char Next(File* file, size_t i) {
  if (i + 1 >= file->len)
    return '\0';

  return file->data[i + 1];
}

void CpyRow(File* file, char* row, size_t offset, size_t eol) {
  if (file->data[offset] == ' ') offset++; // do not render empty space as row first char

  for (size_t k = 0, i = offset; i <= eol && i < file->len; k++) {
    i = offset + k;
    char c = file->data[i];
    if (c == '\n' || c == '\r') c = '\0';
    if (c == '\t') c = ' ';
    row[k] = c;
  }
  row[eol - offset] = '\0';
}

bool IsCharBetween(char c, int a, int b) {
  return c >= a && c <=b;
}

void BuildRows(File *file) {
  size_t rlen = 10;
  C.rows = (char**)malloc(rlen * sizeof(char*));

  unsigned int currentRow = 0;
  size_t lineStart = 0;
  for (size_t i = 0; i < file->len; i++) {

    size_t j = i;
    while (j - lineStart >= LINE_BREAK_SIZE &&
      IsCharBetween(file->data[i], 33, 125) &&
      IsCharBetween(Next(file, i), 33, 125)) {
      i--;
    }

    if (file->data[i] == '\n' || file->data[i] == '\r' || j - lineStart >= LINE_BREAK_SIZE) {
      if (file->data[i] == '\r' && Next(file, i) == '\n') i++;
      C.rows[currentRow] = (char*)malloc(i - lineStart + 1);
      CpyRow(file, C.rows[currentRow], lineStart, i);

      currentRow++;
      lineStart = i + 1;
      if (currentRow >= rlen) {
        rlen *= 2;
        char **tmp = realloc(C.rows, rlen * sizeof(char*));
        C.rows = tmp;
      }
    }
  }

  C.rowslen = currentRow;
}

void LoadCustomFont(void) {
  Font customFont = LoadFontEx("resources/FiraCode-Regular.ttf", C.font.size, NULL, 250); 
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

  C.eMargin = (Vector2) {
    (C.window.width - lineSize.x) / 2,
    C.window.height * 0.07
  };
}

void Init(char* filepath) {
  // DATA INIT
  File file = FileRead(filepath);
  BuildRows(&file);

  // WINDOW INIT
  C.window = (Window){1280, 720};
  InitWindow(C.window.width, C.window.height, "Olive");
  SetWindowState(FLAG_WINDOW_RESIZABLE);

  SetTargetFPS(20);

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
    C.rowoff = MIN(C.rowoff + 1, C.rowslen - 1);  // Mouse wheel down
  }
}

static char lastKey = 0;
static int repeatTime = 0;
void KeyboardHandler(void) {
  if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) {
     C.cx++;
     if (C.cx > strlen(C.rows[C.cy])) {
      C.cx = 0;
      C.cy++;
    }
  }
  if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN)) {
    C.cy = MIN(C.cy+1, C.rowslen-1);
    if (C.cy - C.rowoff >= C.screenrows) {
      C.rowoff = MIN(C.rowoff+1, C.rowslen);
    }

    if (C.cx > strlen(C.rows[C.cy])) {
      C.cx = strlen(C.rows[C.cy]);
    }
  }
  if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) {
    C.cx--;
    if (C.cx < 0) {
      if (C.cy == 0) {
        C.cx = 0;
        return;
      }
      C.cx = strlen(C.rows[MAX(0, C.cy-1)]);
      C.cy = MAX(C.cy - 1, 0);
    } 
  }
  if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)) {
    C.cy = MAX(C.cy-1, 0);
    if (C.cy - C.rowoff <= 0)
      C.rowoff = MAX(C.rowoff-1, 0);

    if (C.cx > strlen(C.rows[C.cy])) {
      C.cx = strlen(C.rows[C.cy]);
    }
  }
}

static float blinkT;
static bool cVisible = true;
void DrawCursor() {
  blinkT += GetFrameTime();

  if (blinkT >= 0.5) {
    blinkT = 0.f;
    cVisible = !cVisible;
  }

  if (!cVisible) return;

  float x = C.cx * C.font.font.recs->width + C.eMargin.x;
  float y = C.font.lineSpacing * (C.cy - C.rowoff) + C.eMargin.y;
  DrawRectangleV((Vector2){x, y}, (Vector2){1, C.font.size}, DARKGRAY);
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

    C.screenrows = 0;
    for (size_t i = 0; i + C.rowoff < C.rowslen; i++) {
      float y = C.font.lineSpacing * i + C.eMargin.y;
      if (y + C.font.size >= C.window.height) break;

      DrawTextEx(C.font.font,
                 C.rows[i + C.rowoff],
                 (Vector2){C.eMargin.x, y},
                 C.font.size,
                 0,
                 DARKGRAY);
      C.screenrows++;
    }

    DrawCursor();

    EndDrawing();
    // === END DRAWING ===
  }

  // == BEGIN De-Initialization ===
  CloseWindow();        // Close window and OpenGL context
  // === END DE-INITIALIZATION ===

  return 0;
}