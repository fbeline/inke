#pragma once

#include <raylib.h>

#include "types.h"
#include "editor.h"

typedef struct render_s {
  i32 margin_top, margin_left, margin_bottom;
  u16 font_size, font_line_spacing;
  u16 window_height, window_width;
  u16 ncol, nrow;
  f32 scale;
  i32 message_box;
  Font font;
} render_t;

void render_draw(editor_t *E, render_t* R);

void render_update_window(render_t* R);

void render_reload_font(render_t* R);

render_t render_init(u16 width, u16 height, u16 font_size);
