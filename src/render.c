#include "render.h"

#include <string.h>
#include "utils.h"

static float blinkT;
static bool cVisible = true;
void DrawCursor(Editor* E) {
  blinkT += GetFrameTime();

  if (blinkT >= 0.5) {
    blinkT = 0.f;
    cVisible = !cVisible;
  }

  if (!cVisible) return;

  f32 x = E->cx * E->font.font.recs->width + E->eMargin.x;
  f32 y = E->font.lineSpacing * (E->cy - E->rowoff) + E->eMargin.y;
  DrawRectangleV((Vector2){x, y}, (Vector2){1, E->font.size}, DARKGRAY);
}

void render_load_font(Editor *E) {
  Font customFont = LoadFontEx("resources/FiraCode-Regular.ttf", E->font.size, NULL, 250); 
  E->font.lineSpacing = E->font.size * 1.15;
  SetTextLineSpacing(E->font.lineSpacing);
  GenTextureMipmaps(&customFont.texture); 
  SetTextureFilter(customFont.texture, TEXTURE_FILTER_BILINEAR);
  E->font.font = customFont;

  Vector2 lineSize = 
    MeasureTextEx(customFont,
                  DUMMY_LINE,
                  E->font.size,
                  0);

  E->eMargin = (Vector2) {
    (E->window.width - lineSize.x) / 2,
    E->window.height * 0.07
  };
}

void render_reload_font(Editor *E) {
  f32 scale = (f32)GetScreenWidth() / E->window.width;
  E->window = (Window) { GetScreenWidth(), GetScreenHeight() };

  UnloadFont(E->font.font);

  E->font.size *= scale;
  render_load_font(E);
}

void render_draw(Editor* E) {
    BeginDrawing();

    ClearBackground(RAYWHITE);

    E->screenrows = 0;
    for (usize i = 0; i + E->rowoff < E->rowslen; i++) {
      f32 y = E->font.lineSpacing * i + E->eMargin.y;
      if (y + E->font.size >= E->window.height) break;

      char vrow[MAX_COL + 1] = "";
      Row row = E->rows[i + E->rowoff];
      usize row_len = strlen(row.chars);
      row_len = row_len > E->coloff ? row_len - E->coloff : 0;
      usize vrow_len = MIN(row_len, MAX_COL);

      if (vrow_len > E->coloff)
        memcpy(vrow, row.chars + E->coloff, vrow_len);

      DrawTextEx(E->font.font,
                 vrow,
                 (Vector2){E->eMargin.x, y},
                 E->font.size,
                 0,
                 DARKGRAY);

      E->screenrows++;
    }

    DrawRectangleV(
      (Vector2){E->eMargin.x + MAX_COL * E->font.font.recs->width, E->eMargin.y},
      (Vector2){3, E->window.height - E->eMargin.y},
      (Color){ 238, 238, 238, 255 }
    );

    DrawCursor(E);

    EndDrawing();
}
