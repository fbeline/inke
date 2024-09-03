#pragma once

#include <raylib.h>

#include "types.h"
#include "cursor.h"
#include "editor.h"

typedef struct render_s {
  i32 margin_top, margin_left, margin_bottom;
  u16 font_size, font_line_spacing;
  u16 window_height, window_width;
  u16 ncol, nrow;
  Font font;
} render_t;

void render_reload_font(render_t* R);

void render_draw(cursor_t* C, render_t* R);

void render_update_window(cursor_t* C, render_t* R);

render_t render_init(u16 width, u16 height, u16 font_size);
