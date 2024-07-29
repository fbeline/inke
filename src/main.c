#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "types.h"
#include "fs.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MAX_COL 66
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

typedef struct row {
  usize size;
  char* chars;
} Row;

typedef struct editor {
  char filename[255];
  int cx, cy;
  int rowoff;
  int coloff;
  int screenrows;
  int screencols;
  Vector2 eMargin;
  EditorFont font;
  unsigned int rowslen;
  unsigned int rowSize;
  Row* rows;
  Window window;
} Editor;

static Editor E = {0};

char Next(File* file, usize i) {
  if (i + 1 >= file->len)
    return '\0';

  return file->data[i + 1];
}

char* RowsToString(Row* rows, unsigned int size) {
  usize strsize = 1;
  usize strl = 0;
  char* str = malloc(strsize);
  str[0] = '\0';

  for (int i = 0; i < size; i++) {
    strsize += rows[i].size + 1;
    char* tmp = realloc(str, strsize);
    if (tmp == NULL) return NULL;
    str = tmp;

    memcpy(str + strl, rows[i].chars, strlen(rows[i].chars));
    strl += strlen(rows[i].chars);
    str[strl] = '\n';
    strl++;
  }
  str[strl] = '\0';

  return str;
}

void CpyRow(File* file, Row* row, usize offset, usize eol) {
  row->size = eol - offset + 2;
  row->chars = (char*)malloc(row->size);

  if (file->data[offset] == ' ') offset++; // do not render empty space as row first char

  for (usize k = 0, i = offset; i <= eol && i < file->len; k++) {
    i = offset + k;
    char c = file->data[i];
    if (c == '\n' || c == '\r') c = '\0';
    if (c == '\t') c = ' ';
    row->chars[k] = c;
  }
  row->chars[row->size - 1] = '\0';
}

bool IsCharBetween(char c, int a, int b) {
  return c >= a && c <=b;
}

void BuildRows(File *file) {
  usize rowSize = 10;
  E.rows = (Row*)malloc(rowSize * sizeof(Row));

  unsigned int currentRow = 0;
  usize lineStart = 0;
  for (usize i = 0; i < file->len; i++) {

    if (file->data[i] == '\n' || file->data[i] == '\r') {
      usize eol = i - 1;
      if (file->data[i] == '\r' && Next(file, i) == '\n') {
        i++;
        eol--;
      }
      CpyRow(file, &E.rows[currentRow], lineStart, i);

      currentRow++;
      lineStart = i + 1;
      if (currentRow >= rowSize) {
        rowSize *= 2;
        Row *tmp = realloc(E.rows, rowSize * sizeof(Row));
        E.rows = tmp;
      }
    }
  }

  E.rowSize = rowSize;
  E.rowslen = currentRow;
}

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

void InsertCharAt(Row* row, int c, int i) {
  u32 len = strlen(row->chars);
  if (i < 0 || i > len) {
    printf("Invalid position\n");
    return;
  }
  
  char* tmp = malloc(len - i + 1);
  if (!tmp) return;

  memcpy(tmp, row->chars + i, len - i);

  row->chars[i] = c;
  memcpy(row->chars + i + 1, tmp, len - i);
  row->chars[len + 1] = '\0';

  free(tmp);
}

void InsertChar(int c) {
  if (strlen(E.rows[E.cy].chars) + 1 >= E.rows[E.cy].size) {
    E.rows[E.cy].size += 8;
    char* tmp = realloc(E.rows[E.cy].chars, E.rows[E.cy].size);
    if (tmp == NULL) {
      printf("MEM ALLOC FAILED\n");
      return;
    }
    E.rows[E.cy].chars = tmp;
  }
  InsertCharAt(&E.rows[E.cy], c, E.cx);
  E.cx++;
}

void RemoveCharAtCursor(void) {
  if (E.cx == 0) return;

  usize len = strlen(E.rows[E.cy].chars);
  for (int i = E.cx-1; i < len-1; i++) {
    E.rows[E.cy].chars[i] = E.rows[E.cy].chars[i + 1];
  }
  E.rows[E.cy].chars[len-1] = '\0';

  E.cx = E.cx - 1;
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

void MouseWheelHandler(void) {
  float mouseWheelMove = GetMouseWheelMove();
  if (mouseWheelMove > 0) {
    E.rowoff = MAX(E.rowoff - 1, 0);
  }
  else if (mouseWheelMove < 0) {
    E.rowoff = MIN(E.rowoff + 1, E.rowslen - 1);  // Mouse wheel down
  }
}

bool InsertRowAt(usize n) {
  usize newLen = E.rowslen + 1;
  if (newLen >= E.rowSize) {
    usize newSize = E.rowSize + 10;
    Row* tmp = (Row*)realloc(E.rows, newSize * sizeof(Row));
    if (tmp == NULL) return false;

    E.rowSize = newSize;
  }

  memmove(E.rows + n + 1, E.rows + n, sizeof(Row) * (E.rowslen - n));
  E.rowslen = newLen;

  E.rows[n] = (Row){0, NULL};
  return true;
}

void ReturnHandler(void) {
  if (!InsertRowAt(E.cy + 1)) return;

  E.rows[E.cy + 1].size = strlen(E.rows[E.cy].chars) - E.cx + 2;
  E.rows[E.cy + 1].chars = malloc(E.rows[E.cy + 1].size);
  memcpy(E.rows[E.cy + 1].chars,
         E.rows[E.cy].chars + E.cx,
         strlen(E.rows[E.cy].chars) - E.cx + 1);

  E.rows[E.cy].chars[E.cx] = '\0';

  E.cy++;
  E.cx = 0;
}

static char lastKey = 0;
static int repeatTime = 0;
void KeyboardHandler(void) {
  if (IsKeyPressed(KEY_ENTER)) {
    ReturnHandler();
    return;
  }
  if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) {
    E.cx++;
    if (E.cx + E.coloff > MAX_COL) {
      E.cx = MAX_COL;
      E.coloff++;
    }
    if (E.cx + E.coloff > strlen(E.rows[E.cy].chars)) {
      E.coloff = 0;
      E.cx = 0;
      E.cy++;
    }
  }
  if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN)) {
    E.cy = MIN(E.cy+1, E.rowslen-1);
    if (E.cy - E.rowoff >= E.screenrows) {
      E.rowoff = MIN(E.rowoff+1, E.rowslen);
    }

    if (E.cx > strlen(E.rows[E.cy].chars)) {
      E.cx = strlen(E.rows[E.cy].chars);
    }
  }
  if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) {
    E.cx--;
  
    if (E.cx < 0) {
      if (E.cy == 0) {
        E.cx = 0;
        return;
      }
      int rowlen = strlen(E.rows[MAX(0, E.cy-1)].chars);
      if (E.coloff == 0) {
        E.cx = MIN(rowlen, MAX_COL);
        E.cy = MAX(E.cy - 1, 0);
        if (rowlen > MAX_COL) E.coloff = rowlen - MAX_COL;
      } else {
        E.coloff--;
        E.cx = 0;
      }
    } 
  }
  if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)) {
    E.cy = MAX(E.cy-1, 0);
    if (E.cy - E.rowoff <= 0)
      E.rowoff = MAX(E.rowoff-1, 0);

    if (E.cx > strlen(E.rows[E.cy].chars)) {
      E.cx = strlen(E.rows[E.cy].chars);
    }
  }
  if (IsKeyPressed(KEY_BACKSPACE)) {
    RemoveCharAtCursor();
  }
  if (IsKeyPressed(KEY_F10)) {
    char *buf = RowsToString(E.rows, E.rowslen);
    FileWrite(E.filename, buf);
    free(buf);
  }

  int ch = GetCharPressed();
  if (IsCharBetween(ch, 32, 127)) InsertChar(ch);
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

  float x = E.cx * E.font.font.recs->width + E.eMargin.x;
  float y = E.font.lineSpacing * (E.cy - E.rowoff) + E.eMargin.y;
  DrawRectangleV((Vector2){x, y}, (Vector2){1, E.font.size}, DARKGRAY);
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

    // === BEGIN DRAWING ===
    BeginDrawing();

    ClearBackground(RAYWHITE);

    E.screenrows = 0;
    for (usize i = 0; i + E.rowoff < E.rowslen; i++) {
      float y = E.font.lineSpacing * i + E.eMargin.y;
      if (y + E.font.size >= E.window.height) break;

      char vrow[MAX_COL + 1] = "";
      Row row = E.rows[i + E.rowoff];
      usize row_len = strlen(row.chars);
      row_len = row_len > E.coloff ? row_len - E.coloff : 0;
      usize vrow_len = MIN(row_len, MAX_COL);

      if (vrow_len > E.coloff)
        memcpy(vrow, row.chars + E.coloff, vrow_len);

      DrawTextEx(E.font.font,
                 vrow,
                 (Vector2){E.eMargin.x, y},
                 E.font.size,
                 0,
                 DARKGRAY);

      E.screenrows++;
    }

    DrawRectangleV(
      (Vector2){E.eMargin.x + MAX_COL * E.font.font.recs->width, E.eMargin.y},
      (Vector2){3, E.window.height - E.eMargin.y},
      (Color){ 238, 238, 238, 255 }
    );

    DrawCursor();

    EndDrawing();
    // === END DRAWING ===
  }

  // == BEGIN De-Initialization ===
  CloseWindow();        // Close window and OpenGL context
  // === END DE-INITIALIZATION ===

  return 0;
}
