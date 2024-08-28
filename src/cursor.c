#include "cursor.h"

#include <string.h>
#include <stdlib.h>
#include "undo.h"
#include "utils.h"
#include <stdio.h>

static cursor_t C = {0};

static i32 raw_x(void) {
  return C.x + C.coloff;
}

static i32 raw_y(void) {
  return C.y + C.rowoff;
}

cursor_t cursor_get(void) {
  return C;
}

void cursor_set(cursor_t* cursor) {
  C.region = cursor->region;
  C.x = cursor->x;
  C.y = cursor->y;
  C.coloff = cursor->coloff;
  C.rowoff = cursor->rowoff;
}

vec2_t cursor_position(void) {
  return (vec2_t) {raw_x(), raw_y()};
}

region_t cursor_region(void) {
  return C.region;
}

void cursor_region_start(void) {
  if (!C.region.active) {
    C.region.active = true;
    C.region.pos = cursor_position();
    C.region.vpos = (vec2_t){C.x, C.y};
  } else {
    cursor_clear_region();
  }
}

char* cursor_region_text(editor_t* E) {
  if (!C.region.active) return NULL;

  vec2_t cp = cursor_position();
  vec2_t rp = C.region.pos;
  vec2_t ps = rp.y <= cp.y ? rp : cp;
  vec2_t pe = rp.y > cp.y ? rp : cp;

  return editor_text_between(E, ps, pe);
}

char* cursor_region_kill(editor_t* E) {
  if (!C.region.active) return NULL;

  vec2_t cp = cursor_position();
  vec2_t rp = C.region.pos;
  vec2_t ps = rp.y <= cp.y ? rp : cp;
  vec2_t pe = rp.y > cp.y ? rp : cp;

  char* txt = editor_cut_between(E, ps, pe);

  C.y -= (pe.y - ps.y);
  C.x = ps.x;

  return txt;
}

void cursor_clear_region(void) {
  C.region.active = false;
}

void cursor_set_max(u16 max_col, u16 max_row) {
  C.max_row = max_row;
  C.max_col = max_col;
}

char cursor_char(editor_t* E) {
  return editor_char_at(E, raw_x(), raw_y());
}

void cursor_bol(void) {
  C.x = 0;
  C.coloff = 0;
}

void cursor_eol(editor_t* E) {
  i32 len = editor_rowlen(E, raw_y());
  if (len > C.max_col) {
    C.x = C.max_col;
    C.coloff = len - C.max_col;
  } else {
    C.x = len;
  }
}

void cursor_down(editor_t* E) {
  vec2_t pos = cursor_position();
  if (pos.y >= E->row_size - 1) return;

  if (C.y < C.max_row) {
    C.y++;
  } else {
    C.rowoff++;
    C.region.vpos.y--;
  }

  if (pos.x > editor_rowlen(E, pos.y + 1)) {
    cursor_eol(E);
  }
}

void cursor_up(editor_t* E) {
  i32 y = raw_y();
  if (y <= 0) return;

  if (C.y == 0 && C.rowoff > 0) {
    C.rowoff--;
    C.region.vpos.y++;
  } else {
    C.y--;
  }

  if (raw_x() > editor_rowlen(E, raw_y())) {
    cursor_eol(E);
  }
}

void cursor_right(editor_t* E) {
  vec2_t pos = cursor_position();
  usize len = editor_rowlen(E, pos.y);

  if (pos.x < len && C.x == C.max_col) {
    C.coloff++;
  } else if (pos.x >= len) {
    cursor_down(E);
    cursor_bol();
  } else {
    C.x++;
  }
}

void cursor_left(editor_t* E) {
  if (C.x == 0 && C.coloff > 0) {
    C.coloff--;
  } else if (C.x == 0 && C.coloff == 0 && (C.y > 0 || C.rowoff > 0)) {
    cursor_up(E);
    cursor_eol(E);
  } else {
    C.x--;
  }
}

void cursor_break_line(editor_t* E) {
  vec2_t pos = cursor_position();
  editor_break_line(E, pos.x, pos.y);
  cursor_down(E);
  cursor_bol();
}

void cursor_move_word_forward(editor_t* E) {
  char ch1, ch2;
  do {
    vec2_t pos = cursor_position();
    if (pos.y >= E->row_size - 1 && editor_rowlen(E, pos.y) >= pos.x)
      break;
    cursor_right(E);
    ch1 = cursor_char(E);
    ch2 = editor_char_at(E, pos.x + 2, raw_y());
  } while(!(ch1 !=  ' ' && ch2 == ' '));
  cursor_right(E);
}

void cursor_move_word_backward(editor_t* E) {
  char ch1, ch2;
  do {
    vec2_t pos = cursor_position();
    if (pos.y == 0 && pos.x == 0)
      break;

    cursor_left(E);
    ch1 = cursor_char(E);
    ch2 = editor_char_at(E, pos.x - 2, raw_y());
  } while(!(ch1 !=  ' ' && ch2 == ' '));
}

void cursor_remove_char(editor_t *E) {
  vec2_t pos = cursor_position();

  if (pos.x == 0) {
    usize len = editor_rowlen(E, pos.y - 1);
    editor_move_line_up(E, pos.y);
    cursor_up(E);
    cursor_eol(E);
    return;
  }

  vec2_t undo_pos = {pos.x - 1, pos.y};
  char strdata[2] = { editor_char_at(E, undo_pos.x, undo_pos.y), '\0' };
  undo_push(BACKSPACE, undo_pos, cursor_get(), strdata);

  editor_delete_char_at(E, pos);

  if (C.x == 0 && C.coloff > 0)
    C.coloff--;
  else
    C.x--;

  E->dirty = true;
}

void cursor_insert_char(editor_t* E, int ch) {
  editor_insert_char_at(E, raw_x(), raw_y(), ch);

  undo_push(ADD,
            (vec2_t){raw_x() + 1, raw_y()},
            cursor_get(),
            NULL);

  C.x++;
  if (C.x > C.max_col) {
    C.coloff++;
    C.x = C.max_col;
  }

  E->dirty = true;
}

void cursor_insert_text(editor_t* E, const char* text) {
  usize len = strlen(text);
  for (usize i = 0; i < len; i++) {
    if (text[i] == '\n') {
      cursor_break_line(E);
    } else {
      cursor_insert_char(E, text[i]);
    }
  }
}

void cursor_page_up(editor_t* E) {
  for (i32 i = 0; i < C.max_row; i ++) {
    cursor_up(E);
  }
}

void cursor_page_down(editor_t* E) {
  for (i32 i = 0; i < C.max_row; i ++) {
    cursor_down(E);
  }
}

void cursor_delete_forward(editor_t* E) {
  editor_delete_forward(E, raw_x(), raw_y());
}

void cursor_delete_row(editor_t* E) {
  i32 n = raw_y();
  editor_delete_rows(E, n, n);
}

void cursor_eof(editor_t* E) {
  if (E->row_size > C.max_row) {
    C.y = C.max_row - 1;
    C.rowoff = E->row_size - C.max_row;
  } else {
    C.y = E->row_size;
    C.rowoff = 0;
  }
  cursor_eol(E);
}

void cursor_bof(void) {
  cursor_bol();
  C.y = 0;
  C.rowoff = 0;
}
