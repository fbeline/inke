#include "render.h"

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include <string.h>
#include <stdio.h>

#include "editor.h"
#include "fs.h"
#include "utils.h"

static f32 blinkT;
static bool cVisible = true;
static const f32 margin_p = 0.05;

static void render_draw_cursor(editor_t* E, render_t* R) {
  blinkT += GetFrameTime();

  if (blinkT >= 0.5) {
    blinkT = 0.f;
    cVisible = !cVisible;
  }

  if (!cVisible) return;

  f32 x = E->cx * R->font.recs->width + R->margin_left;
  f32 y = R->font_line_spacing * (E->cy - E->rowoff) + R->margin_top;
  DrawRectangleV((Vector2){x, y}, (Vector2){1, R->font_size}, DARKGRAY);
}

static void draw_status_bar(editor_t* E, render_t* R) {
  i32 font_size = R->font_size / 2;
  char row_col[15];
  sprintf(row_col, "%d/%d", E->cy + 1, E->cx + 1);
  Vector2 row_col_size = MeasureTextEx(GetFontDefault(), "00000000/000", font_size, 0.0f);

  i32 xpos = R->window_width - row_col_size.x - R->margin_left;
  i32 ypos = R->window_height - row_col_size.y;

  DrawRectangle(0, ypos - 2, R->window_width, font_size + 2, RAYWHITE);

  DrawTextEx(R->font,
             row_col,
             (Vector2){xpos, ypos},
             font_size,
             0,
             DARKGRAY);

  char filename[260];
  const usize filename_len = strlen(E->filename);

  memcpy(filename, E->filename, filename_len);
  filename[filename_len] = '\0';
  if (E->dirty) {
    memcpy(filename + filename_len, " [+]\0", 5);
  }

  DrawTextEx(R->font,
             filename,
             (Vector2){R->margin_left, ypos},
             font_size,
             0,
             DARKGRAY);
}

static void render_load_font(u16 font_size, render_t* R) {
  R->font_size = font_size;
  R->font_line_spacing = R->font_size * 1.15;
  R->font = LoadFontEx("resources/FiraCode-Regular.ttf", R->font_size, NULL, 250);

  SetTextLineSpacing(R->font_line_spacing);
  GenTextureMipmaps(&R->font.texture); 
  SetTextureFilter(R->font.texture, TEXTURE_FILTER_BILINEAR);

  const f64 gui_font_size = MAX(10.f, GuiGetFont().baseSize * R->scale); 
  GuiSetStyle(DEFAULT, TEXT_SIZE, gui_font_size);

  Vector2 line_size = MeasureTextEx(R->font, DUMMY_LINE, R->font_size, 0);
  R->margin_top = R->window_height * margin_p;
  R->margin_bottom = R->window_height * margin_p;
  R->margin_left = (R->window_width - line_size.x) / 2;
}

static void render_draw_info(editor_t* E, render_t* R) {
  char info[11] = {0};
  sprintf(info, "OLIVE v%s", VERSION);
  int text_size = MeasureText(info, R->font_size);

  DrawTextEx(R->font, 
             info, 
             (Vector2) {R->window_width/2.f - text_size/2.f, R->window_height/2.f}, 
             R->font_size, 
             0, 
             DARKGRAY);

}

static void render_draw_vertical_bar(editor_t* E, render_t* R) {
  DrawRectangleV(
    (Vector2){R->margin_left + MAX_COL * R->font.recs->width, R->margin_top},
    (Vector2){3, R->window_height - R->margin_top},
    (Color){ 238, 238, 238, 255 }
  );
}

static void render_draw_lines(editor_t* E, render_t* R) {
  E->screenrows = 0;
  for (usize i = 0; i + E->rowoff < E->row_size; i++) {
    f32 y = R->font_line_spacing * i + R->margin_top;

    if (i >= MAX_ROW) break;

    char vrow[MAX_COL + 1] = {0};
    row_t row = E->rows[i + E->rowoff];
    i64 row_len = strlen(row.chars);
    i64 vrow_len = MIN(row_len - E->coloff, MAX_COL);
    if (vrow_len <= 0) continue;

    memcpy(vrow, row.chars + E->coloff, vrow_len);
    DrawTextEx(R->font,
               vrow,
               (Vector2){R->margin_left, y},
               R->font_size,
               0,
               DARKGRAY);

    E->screenrows++;
  }

}

static void render_draw_message_box(editor_t* E, render_t* R) {
  if (R->message_box == 0) return;

  f32 box_width = 350 * MAX(R->scale * 0.7, 1.f);
  f32 box_height = 150 * MAX(R->scale * 0.7, 1.f);
  f32 box_x = (R->window_width - box_width) / 2;
  f32 box_y = (R->window_height - box_height) / 2;

  int result = GuiMessageBox(
    (Rectangle){ box_x, box_y, box_width, box_height },
    "#191#Save Changes?", 
    "Open documents contain unsaved changes.", 
    "Cancel;Discard;Save");

  char* buf = NULL;
  switch (result) {
    case 1:
      R->message_box = 0;
      break;
    case 2:
      E->running = false;
      break;
    case 3:
      buf = editor_rows_to_string(E->rows, E->row_size);
      FileWrite(E->filename, buf);
      free(buf);
      E->running = false;
  }
}

void render_reload_font(render_t* R) {
  if (GetScreenWidth() == R->window_width) return;

  R->scale = (f32)GetScreenWidth() / R->window_width;
  R->window_width = GetScreenWidth();
  R->window_height = GetScreenHeight();

  UnloadFont(R->font);

  render_load_font(R->font_size * R->scale, R);
}

void render_draw(editor_t* E, render_t* R) {
  BeginDrawing();

  ClearBackground(RAYWHITE);

  if (E->new_file && !E->dirty) {
    render_draw_info(E, R);
  }
  else {
    render_draw_lines(E, R);
    render_draw_vertical_bar(E, R);
    render_draw_cursor(E, R);

    render_draw_message_box(E, R);
  }

  draw_status_bar(E, R);

  EndDrawing();
}

render_t render_init(u16 width, u16 height, u16 font_size) {
  render_t R;
  R.scale = 1;
  R.message_box = 0;
  R.window_width = width;
  R.window_height = height;

  InitWindow(width, height, "Olive");
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);
  SetExitKey(KEY_NULL);

  render_load_font(font_size, &R);

  return R;
}
