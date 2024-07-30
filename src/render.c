#include "render.h"

#include <string.h>
#include "utils.h"

#include "editor.h"

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

void render_draw(void) {
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
}
