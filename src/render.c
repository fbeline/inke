#include "render.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "editor.h"
#include "mode.h"
#include "cursor.h"
#include "utils.h"

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

static void render_draw_region(cursor_t* C, render_t* R) {
  if (!C->region.active) return;

  vec2_t cp = {C->x, C->y};
  vec2_t rp = C->region.vpos;
  vec2_t ps = rp.y <= cp.y ? rp : cp;
  vec2_t pe = rp.y > cp.y ? rp : cp;
  pe.y = MIN((i32)R->nrow + 1, pe.y);
  for (i32 i = ps.y; i <= pe.y; i++) {
    i32 xs = i == ps.y ? ps.x : 0;
    i32 xe = i == pe.y ? pe.x : editor_rowlen(C->editor, i + C->rowoff);
    render_highlight_line(R, i, xs, xe);
  }
}

static void render_draw_cursor(cursor_t* C, render_t* R) {
  if (!(g_mode & MODE_INSERT)) return;

  Vector2 pos = render_position(R, C->x, C->y);
  DrawRectangleV(pos,
                 (Vector2){R->font.recs->width, R->font.recs->height},
                 DARKGRAY);

  char chc[2] = {'\0', '\0'};
  chc[0] = cursor_char(C);

  // draw font
  DrawTextEx(R->font, chc, pos, R->font_size, 0, RAYWHITE);
}

static void draw_command_bar(render_t* R) {
  if (!(g_mode & (COMMAND_CHAIN | COMMAND_SINGLE_CHAR)))
    return;

  i32 ypos = R->window_height - R->font.recs->height;
  DrawTextEx(R->font,
             g_active_command.text,
             (Vector2){R->margin_left, ypos},
             R->font_size,
             0,
             DARKGRAY);
}

static void draw_message(render_t* R) {
  const char* message = mode_get_message();
  if (strlen(message) == 0 || !(g_mode & MODE_INSERT))
    return;

  i32 ypos = R->window_height - R->font.recs->height;
  DrawTextEx(R->font,
             message,
             (Vector2){R->margin_left, ypos},
             R->font_size,
             0,
             DARKGRAY);
}

static void draw_status_bar(cursor_t* C, render_t* R) {
  char row_col[15];
  vec2_t cpos = cursor_position(C);
  sprintf(row_col, "%d/%d", cpos.y + 1, cpos.x + 1);
  Vector2 row_col_size = MeasureTextEx(GetFontDefault(), "00000000/000", R->font_size, 0.0f);

  i32 xpos = R->window_width - row_col_size.x - R->margin_left;
  i32 ypos = R->window_height - (row_col_size.y * 2);

  DrawRectangle(0, ypos - 2, R->window_width, R->font_size + 2, DARKGRAY);

  DrawTextEx(R->font,
             row_col,
             (Vector2){xpos, ypos},
             R->font_size,
             0,
             RAYWHITE);

  char filename[260];
  const usize filename_len = strlen(C->editor->filename);

  memcpy(filename, C->editor->filename, filename_len);
  filename[filename_len] = '\0';
  if (C->editor->dirty) {
    memcpy(filename + filename_len, " [+]\0", 5);
  }

  DrawTextEx(R->font,
             filename,
             (Vector2){R->margin_left, ypos},
             R->font_size,
             0,
             RAYWHITE);
}

static void render_load_font(u16 font_size, render_t* R) {
  char path[512];
  R->font_size = font_size;
  R->font_line_spacing = R->font_size * 1.15;

#ifdef _WIN32
  snprintf(path, sizeof(path), "resources/FiraCode-Regular.ttf");
#else
  const char *home = getenv("HOME");
  if (home == NULL) die("Cannot find $HOME.");

  snprintf(path, sizeof(path), "%s/.fonts/FiraCode-Regular.ttf", home);
#endif

  R->font = LoadFontEx(path, R->font_size, NULL, 250);
  SetTextLineSpacing(R->font_line_spacing);
  GenTextureMipmaps(&R->font.texture);
  SetTextureFilter(R->font.texture, TEXTURE_FILTER_BILINEAR);
}

static void render_draw_info(render_t* R) {
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

static void render_draw_vertical_bar(render_t* R) {
  DrawRectangleV(
    (Vector2){R->margin_left + 80 * R->font.recs->width, R->margin_top},
    (Vector2){3, R->window_height - R->margin_top},
    (Color){ 238, 238, 238, 255 }
  );
}

static void render_draw_lines(cursor_t* C, render_t* R) {
  editor_t* E = C->editor;
  for (usize i = 0; i + C->rowoff < E->row_size; i++) {
    f32 y = R->font_line_spacing * i + R->margin_top;

    if (i > R->nrow) break;

    char vrow[512] = {0};
    row_t row = E->rows[i + C->rowoff];
    i64 row_len = strlen(row.chars);
    i64 vrow_len = MIN(row_len - C->coloff, (i32)R->ncol);

    if (vrow_len <= 0)
      continue;

    memcpy(vrow, row.chars + C->coloff, vrow_len);
    DrawTextEx(R->font,
               vrow,
               (Vector2){R->margin_left, y},
               R->font_size,
               0,
               DARKGRAY);
  }
}

void render_reload_font(render_t* R) {
  UnloadFont(R->font);
  render_load_font(R->font_size, R);
}

void render_update_window(cursor_t* C, render_t* R) {
  if (R == NULL ||
    (GetScreenWidth() == R->window_width && GetScreenHeight() == R->window_height))
    return;

  R->window_width = GetScreenWidth();
  R->window_height = GetScreenHeight();

  R->ncol = (u16)floor((R->window_width - R->margin_left * 2) / R->font.recs->width);
  R->nrow = (u16)floor((R->window_height - R->margin_top - R->margin_bottom) / (R->font.recs->height * 1.2));

  cursor_set_max(C, R->ncol, R->nrow);
}

void render_draw(cursor_t* C, render_t* R) {
  BeginDrawing();

  ClearBackground(RAYWHITE);

  if (C->editor->new_file && !C->editor->dirty) {
    render_draw_info(R);
  }
  else {
    render_draw_region(C, R);
    render_draw_lines(C, R);
    render_draw_vertical_bar( R);
    render_draw_cursor(C, R);
  }

  draw_status_bar(C, R);
  draw_message(R);
  draw_command_bar(R);

  EndDrawing();
}

render_t render_init(u16 width, u16 height, u16 font_size) {
  render_t R;
  R.margin_top = 20;
  R.margin_bottom = 30;
  R.margin_left = 20;

  InitWindow(width, height, "Olive");
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);
  SetExitKey(KEY_NULL);

  render_load_font(font_size, &R);

  return R;
}
