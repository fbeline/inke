#include "render.h"

#include <string.h>
#include "utils.h"

static render_state_t rs = { 0 };

static f32 blinkT;
static bool cVisible = true;
static void DrawCursor(editor_t* E) {
  blinkT += GetFrameTime();

  if (blinkT >= 0.5) {
    blinkT = 0.f;
    cVisible = !cVisible;
  }

  if (!cVisible) return;

  f32 x = E->cx * rs.font.recs->width + rs.margin_left;
  f32 y = rs.font_line_spacing * (E->cy - E->rowoff) + rs.margin_top;
  DrawRectangleV((Vector2){x, y}, (Vector2){1, rs.font_size}, DARKGRAY);
}

void render_load_font(u16 font_size) {
  rs.font_size = font_size;
  rs.font_line_spacing = rs.font_size * 1.15;
  rs.font = LoadFontEx("resources/FiraCode-Regular.ttf", rs.font_size, NULL, 250);

  SetTextLineSpacing(rs.font_line_spacing);
  GenTextureMipmaps(&rs.font.texture); 
  SetTextureFilter(rs.font.texture, TEXTURE_FILTER_BILINEAR);

  Vector2 line_size = MeasureTextEx(rs.font, DUMMY_LINE, rs.font_size, 0);
  rs.margin_top = rs.window_height * 0.07;
  rs.margin_left = (rs.window_width - line_size.x) / 2;
}

void render_reload_font(void) {
  if (GetScreenWidth() == rs.window_width) return;

  f32 scale = (f32)GetScreenWidth() / rs.window_width;
  rs.window_width = GetScreenWidth();
  rs.window_height = GetScreenHeight();

  UnloadFont(rs.font);

  render_load_font(rs.font_size * scale);
}

void render_draw(editor_t* E) {
  BeginDrawing();

  ClearBackground(RAYWHITE);

  E->screenrows = 0;
  for (usize i = 0; i + E->rowoff < E->rowslen; i++) {
    f32 y = rs.font_line_spacing * i + rs.margin_top;
    if (y + rs.font_size >= rs.window_height) break;

    char vrow[MAX_COL + 1] = {0};
    row_t row = E->rows[i + E->rowoff];
    i64 row_len = strlen(row.chars);
    i64 vrow_len = MIN(row_len - E->coloff, MAX_COL);
    if (vrow_len <= 0) continue;

    memcpy(vrow, row.chars + E->coloff, vrow_len);
    DrawTextEx(rs.font,
               vrow,
               (Vector2){rs.margin_left, y},
               rs.font_size,
               0,
               DARKGRAY);

    E->screenrows++;
  }

  DrawRectangleV(
    (Vector2){rs.margin_left + MAX_COL * rs.font.recs->width, rs.margin_top},
    (Vector2){3, rs.window_height - rs.margin_top},
    (Color){ 238, 238, 238, 255 }
  );

  DrawCursor(E);

  EndDrawing();
}

void render_init(u16 width, u16 height, u16 font_size) {
  rs.window_width = width;
  rs.window_height = height;

  InitWindow(width, height, "Olive");
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);

  render_load_font(font_size);
}
