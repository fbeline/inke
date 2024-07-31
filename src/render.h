#pragma once

#include <raylib.h>

#include "types.h"
#include "editor.h"

typedef struct {
  i32 margin_top, margin_left;
  u16 font_size, font_line_spacing;
  u16 window_height, window_width;
  Font font;
} render_state_t;

void render_draw(Editor *E);

void render_load_font(u16 font_size);

void render_reload_font(void);

void render_init(u16 width, u16 height, u16 font_size);
