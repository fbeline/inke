#include "render.h"

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include <string.h>
#include <stdio.h>

#include "utils.h"

static render_state_t rs = { 0 };

static f32 blinkT;
static bool cVisible = true;
static const f32 margin_p = 0.05;

static void render_draw_cursor(editor_t* E) {
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

static void draw_status_bar(editor_t* E) {
  i32 font_size = rs.font_size / 2;
  char row_col[15];
  sprintf(row_col, "%d/%d", E->cy + 1, E->cx + 1);
  Vector2 row_col_size = MeasureTextEx(GetFontDefault(), "00000000/000", font_size, 0.0f);

  i32 xpos = rs.window_width - row_col_size.x - rs.margin_left;
  i32 ypos = rs.window_height - row_col_size.y;

  DrawRectangle(0, ypos - 2, rs.window_width, font_size + 2, RAYWHITE);

  DrawTextEx(rs.font,
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

  DrawTextEx(rs.font,
             filename,
             (Vector2){rs.margin_left, ypos},
             font_size,
             0,
             DARKGRAY);
}

void render_load_font(u16 font_size) {
  rs.font_size = font_size;
  rs.font_line_spacing = rs.font_size * 1.15;
  rs.font = LoadFontEx("resources/FiraCode-Regular.ttf", rs.font_size, NULL, 250);

  SetTextLineSpacing(rs.font_line_spacing);
  GenTextureMipmaps(&rs.font.texture); 
  SetTextureFilter(rs.font.texture, TEXTURE_FILTER_BILINEAR);

  const f64 gui_font_size = MAX(10.f, GuiGetFont().baseSize * rs.scale); 
  GuiSetStyle(DEFAULT, TEXT_SIZE, gui_font_size);

  Vector2 line_size = MeasureTextEx(rs.font, DUMMY_LINE, rs.font_size, 0);
  rs.margin_top = rs.window_height * margin_p;
  rs.margin_bottom = rs.window_height * margin_p;
  rs.margin_left = (rs.window_width - line_size.x) / 2;
}

void render_reload_font(void) {
  if (GetScreenWidth() == rs.window_width) return;

  rs.scale = (f32)GetScreenWidth() / rs.window_width;
  rs.window_width = GetScreenWidth();
  rs.window_height = GetScreenHeight();

  UnloadFont(rs.font);

  render_load_font(rs.font_size * rs.scale);
}

void render_draw_info(editor_t* E) {
  char info[11] = {0};
  sprintf(info, "OLIVE v%s", VERSION);
  int text_size = MeasureText(info, rs.font_size);

  DrawTextEx(rs.font, 
             info, 
             (Vector2) {rs.window_width/2.f - text_size/2.f, rs.window_height/2.f}, 
             rs.font_size, 
             0, 
             DARKGRAY);

}

void render_draw_vertical_bar(editor_t* E) {
  DrawRectangleV(
    (Vector2){rs.margin_left + MAX_COL * rs.font.recs->width, rs.margin_top},
    (Vector2){3, rs.window_height - rs.margin_top},
    (Color){ 238, 238, 238, 255 }
  );
}

void render_draw_lines(editor_t* E) {
  E->screenrows = 0;
  for (usize i = 0; i + E->rowoff < E->row_size; i++) {
    f32 y = rs.font_line_spacing * i + rs.margin_top;

    if (i >= MAX_ROW) break;

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

}

void render_draw_message_box(editor_t* E) {
  if (rs.message_box == 0) return;

  f32 box_width = 350 * MAX(rs.scale * 0.7, 1.f);
  f32 box_height = 150 * MAX(rs.scale * 0.7, 1.f);
  f32 box_x = (rs.window_width - box_width) / 2;
  f32 box_y = (rs.window_height - box_height) / 2;

  int result = GuiMessageBox(
    (Rectangle){ box_x, box_y, box_width, box_height },
    "#191#Save Changes?", 
    "Open documents contain unsaved changes.", 
    "Cancel;Discard;Save");

  if (result >= 0)
    rs.message_box = 0;
}

void render_draw(editor_t* E) {
  BeginDrawing();

  ClearBackground(RAYWHITE);

  if (E->new_file && !E->dirty) {
    render_draw_info(E);
  }
  else {
    render_draw_lines(E);
    render_draw_vertical_bar(E);
    render_draw_cursor(E);

    render_draw_message_box(E);
  }

  draw_status_bar(E);

  EndDrawing();
}

void render_init(u16 width, u16 height, u16 font_size) {
  rs.scale = 1;
  rs.message_box = 0;
  rs.window_width = width;
  rs.window_height = height;

  InitWindow(width, height, "Olive");
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);
  SetExitKey(KEY_NULL);

  render_load_font(font_size);
}
