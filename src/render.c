#include "render.h"

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include <string.h>
#include <stdio.h>

#include "editor.h"
#include "cursor.h"
#include "fs.h"
#include "utils.h"

static f32 blinkT;
static bool cVisible = true;
static const f32 margin_p = 0.05;

Vector2 render_position(render_t* R, i32 col, i32 row) {
  return (Vector2) {
    col * R->font.recs->width + R->margin_left,
    row * R->font_line_spacing + R->margin_top // TODO: REFACTOR Y
  };
}

static void render_highlight_line(render_t* R, i32 y, i32 xs, i32 xe) {
    Vector2 sp = render_position(R, xs, y);
    Vector2 ep = render_position(R, xe, y);
    DrawRectangle(sp.x, sp.y, ep.x - sp.x, R->font.recs->height, YELLOW);
}

static void render_draw_region(editor_t* E, render_t* R) {
  vec2_t rp = cursor_region();
  if (rp.x == -1 || rp.y == -1)
    return;

  vec2_t cp = cursor_position();
  vec2_t ps = rp.y <= cp.y ? rp : cp;
  vec2_t pe = rp.y > cp.y ? rp : cp;
  for (i32 i = ps.y; i <= pe.y; i++) {
    i32 xs = i == ps.y ? ps.x : 0;
    i32 xe = i == pe.y ? pe.x : editor_rowlen(E, i);
    render_highlight_line(R, i, xs, xe);
  }
}

static void render_draw_cursor(editor_t* E, render_t* R) {
  blinkT += GetFrameTime();

  if (blinkT >= 0.5) {
    blinkT = 0.f;
    cVisible = !cVisible;
  }

  if (!cVisible) return;

  cursor_t cursor = cursor_get();
  Vector2 pos = render_position(R, cursor.x, cursor.y);
  DrawRectangleV(pos,
                 (Vector2){R->font.recs->width, R->font.recs->height},
                 DARKGRAY);

  char chc[2] = {'\0', '\0'};
  chc[0] = cursor_char(E);

  // draw font
  DrawTextEx(R->font, chc, pos, R->font_size, 0, RAYWHITE);
}

static void draw_status_bar(editor_t* E, render_t* R) {
  char row_col[15];
  vec2_t cpos = cursor_position();
  sprintf(row_col, "%d/%d", cpos.y + 1, cpos.x + 1);
  Vector2 row_col_size = MeasureTextEx(GetFontDefault(), "00000000/000", R->font_size, 0.0f);

  i32 xpos = R->window_width - row_col_size.x - R->margin_left;
  i32 ypos = R->window_height - row_col_size.y;

  DrawRectangle(0, ypos - 2, R->window_width, R->font_size + 2, RAYWHITE);

  DrawTextEx(R->font,
             row_col,
             (Vector2){xpos, ypos},
             R->font_size,
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
             R->font_size,
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
    (Vector2){R->margin_left + 80 * R->font.recs->width, R->margin_top},
    (Vector2){3, R->window_height - R->margin_top},
    (Color){ 238, 238, 238, 255 }
  );
}

static void render_draw_lines(editor_t* E, render_t* R) {
  cursor_t cursor = cursor_get();

  for (usize i = 0; i + cursor.rowoff < E->row_size; i++) {
    f32 y = R->font_line_spacing * i + R->margin_top;

    if (i > R->nrow) break;

    char vrow[512] = {0};
    row_t row = E->rows[i + cursor.rowoff];
    i64 row_len = strlen(row.chars);
    i64 vrow_len = MIN(row_len - cursor.coloff, (i32)R->ncol);

    if (vrow_len <= 0)
      continue;

    memcpy(vrow, row.chars + cursor.coloff, vrow_len);
    DrawTextEx(R->font,
               vrow,
               (Vector2){R->margin_left, y},
               R->font_size,
               0,
               DARKGRAY);
  }
}

static void render_draw_message_box(editor_t* E, render_t* R) {
  if (R->message_box == 0) return;

  f32 box_width = 350;
  f32 box_height = 150;
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
  UnloadFont(R->font);
  render_load_font(R->font_size, R);
}

void render_update_window(render_t* R) {
  if (R == NULL || 
    (GetScreenWidth() == R->window_width && GetScreenHeight() == R->window_height))
    return;

  R->window_width = GetScreenWidth();
  R->window_height = GetScreenHeight();

  R->ncol = (u16)floor((R->window_width - R->margin_left * 2) / R->font.recs->width);
  R->nrow = (u16)floor((R->window_height - R->margin_top - R->margin_bottom) / (R->font.recs->height * 1.2));

  cursor_set_max(R->ncol, R->nrow);
}

void render_draw(editor_t* E, render_t* R) {
  BeginDrawing();

  ClearBackground(RAYWHITE);

  if (E->new_file && !E->dirty) {
    render_draw_info(E, R);
  }
  else {
    render_draw_region(E, R);
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
  R.message_box = 0;
  R.margin_top = 20;
  R.margin_bottom = 30;
  R.margin_left = 20;

  InitWindow(width, height, "Olive");
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);
  SetExitKey(KEY_NULL);

  render_load_font(font_size, &R);

  cursor_clear_region();

  return R;
}
