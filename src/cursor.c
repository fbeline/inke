#include "cursor.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "undo.h"
#include "utils.h"

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
  vec2_t pos = cursor_position();
  if (pos.x == 0 && pos.y == 0) {
    return;
  } else if (pos.x == 0 && pos.y > 0) {
    cursor_up(E);
    cursor_eol(E);
  } else if (C.x == 0 && C.coloff > 0) {
    C.coloff--;
  } else {
    C.x--;
  }
}

void cursor_break_line(editor_t* E) {
  vec2_t pos = cursor_position();

  undo_push(LINEBREAK, (vec2_t){0, pos.y+1}, cursor_get(), NULL);

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

  if (pos.x == 0 && pos.y == 0) return;
  if (pos.x == 0 && pos.y > 0) {
    usize prlen = editor_rowlen(E, pos.y-1);
    undo_push(LINEUP, (vec2_t){prlen, pos.y-1}, cursor_get(), NULL);
    editor_move_line_up(E, pos.y);
    cursor_up(E);
    C.x = MIN(C.max_col, prlen);
    C.coloff = MAX(0, (i32)prlen - C.x);
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
  vec2_t pos = cursor_position();
  editor_insert_char_at(E, pos.x, pos.y, ch);

  undo_push(ADD,
            (vec2_t){pos.x + 1, pos.y},
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
  if (text == NULL) return;

  char* flt = NULL;
  usize len = strlen(text);
  usize start = 0;
  usize i = 0;
  usize n = 0;

  for (i = 0; i < len; i++) {
    if (text[i] == '\n') {
      vec2_t pos = cursor_position();
      if (n == 0) {
        flt = strdup(E->rows[pos.y].chars + pos.x);
        E->rows[pos.y].chars[pos.x] = '\0';
      }
      if(text[start] == '\n') start++;

      editor_insert_text(E, pos, text + start, i - start);
      editor_insert_row_at(E, pos.y + 1);

      cursor_down(E);
      cursor_bol();

      n++;
      start = i;
    }
  }

  if (start < i) {
    if(text[start] == '\n') start++;
    editor_insert_text(E, cursor_position(), text + start, i - start);

    for (usize j = start; j < i; j++)
      cursor_right(E);

    if (flt != NULL) {
      editor_insert_text(E, cursor_position(), flt, strlen(flt));
      free(flt);
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
  vec2_t pos = cursor_position();
  usize len = editor_rowlen(E, pos.y);
  const char* strdata = editor_text_between(E, pos, (vec2_t){len, pos.y});
  undo_push(DELETE_FORWARD, pos, cursor_get(), strdata);

  editor_delete_forward(E, pos.x, pos.y);
}

void cursor_delete_row(editor_t* E) {
  i32 y = raw_y();
  bool ll = E->row_size - (y + 1) == 0;

  if (y == 0 && ll) { // editor must have at least one row
    E->rows[0].chars[0] = '\0';
    cursor_bol();
    return;
  }


  char* strdata = strdup(E->rows[y].chars);
  undo_push(LINEDELETE, (vec2_t){0, y}, cursor_get(), strdata);

  editor_delete_rows(E, y, y);

  if (ll) {
    cursor_up(E);
    cursor_bol();
  }
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
