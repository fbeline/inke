#include "cursor.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "undo.h"
#include "utils.h"

static i32 raw_x(cursor_t* C) {
  return C->x + C->coloff;
}

static i32 raw_y(cursor_t* C) {
  return C->y + C->rowoff;
}

void cursor_set(cursor_t* dest, cursor_t* src) {
  dest->region = src->region;
  dest->x = src->x;
  dest->y = src->y;
  dest->coloff = src->coloff;
  dest->rowoff = src->rowoff;
}

vec2_t cursor_position(cursor_t* cursor) {
  return (vec2_t) {raw_x(cursor), raw_y(cursor)};
}

void cursor_region_start(cursor_t* C) {
  if (!C->region.active) {
    C->region.active = true;
    C->region.vpos = (vec2_t){C->x, C->y};
    memcpy(C->region.cursor, C, sizeof(cursor_t));
  } else {
    cursor_clear_region(C);
  }
}

static void cursor_region_calc(cursor_t* C, vec2_t* start, vec2_t* end, cursor_t* result) {
  if (start == NULL || end == NULL) die("Error on region calc\n");

  vec2_t cp = cursor_position(C);
  vec2_t rp = { C->region.cursor->x + C->region.cursor->coloff,
                C->region.cursor->y + C->region.cursor->rowoff };

  if (rp.y <= cp.y) {
    *start = rp;
    *end = cp;
    if (result) *result = *C->region.cursor;
  } else {
    *start = cp;
    *end = rp;
    if (result) *result = *C;
  }
}

char* cursor_region_text(cursor_t* C) {
  if (!C->region.active) return NULL;

  vec2_t ps, pe;
  cursor_region_calc(C, &ps, &pe, NULL);

  line_t *lp = editor_text_between(C->editor, ps, pe);
  char *result = strdup(lp->text);
  line_free(lp);
  return result;
}

char* cursor_region_kill(cursor_t* C) {
  if (!C->region.active) return NULL;

  vec2_t ps, pe;
  cursor_t cursor = {0};
  cursor_region_calc(C, &ps, &pe, &cursor);

  line_t *lp = editor_cut_between(C->editor, ps, pe);
  char* strdata = strdup(lp->text);
  line_free(lp);

  undo_push(CUT, ps, cursor, strdata);
  cursor_set(C, &cursor);

  return strdata;
}

void cursor_clear_region(cursor_t* C) {
  C->region.active = false;
}

void cursor_set_max(cursor_t* C, u16 max_col, u16 max_row) {
  C->max_row = max_row;
  C->max_col = max_col;
}

char cursor_char(cursor_t* C) {
  return editor_char_at(C->editor, raw_x(C), raw_y(C));
}

void cursor_bol(cursor_t* C) {
  C->x = 0;
  C->coloff = 0;
}

void cursor_eol(cursor_t* C) {
  i32 len = editor_rowlen(C->editor, raw_y(C));
  if (len > C->max_col) {
    C->x = C->max_col;
    C->coloff = len - C->max_col;
  } else {
    C->x = len;
  }
}

void cursor_down(cursor_t* C) {
  vec2_t pos = cursor_position(C);
  if (pos.y >= C->editor->row_size - 1) return;

  if (C->y < C->max_row) {
    C->y++;
  } else {
    C->rowoff++;
    C->region.vpos.y--;
  }

  if (pos.x > editor_rowlen(C->editor, pos.y + 1)) {
    cursor_eol(C);
  }
}

void cursor_up(cursor_t* C) {
  i32 y = raw_y(C);
  if (y <= 0) return;

  if (C->y == 0 && C->rowoff > 0) {
    C->rowoff--;
    C->region.vpos.y++;
  } else {
    C->y--;
  }

  if (raw_x(C) > editor_rowlen(C->editor, raw_y(C))) {
    cursor_eol(C);
  }
}

void cursor_right(cursor_t* C) {
  vec2_t pos = cursor_position(C);
  usize len = editor_rowlen(C->editor, pos.y);

  if (pos.x < len && C->x == C->max_col) {
    C->coloff++;
  } else if (pos.x >= len) {
    cursor_down(C);
    cursor_bol(C);
  } else {
    C->x++;
  }
}

void cursor_left(cursor_t* C) {
  vec2_t pos = cursor_position(C);
  if (pos.x == 0 && pos.y == 0) {
    return;
  } else if (pos.x == 0 && pos.y > 0) {
    cursor_up(C);
    cursor_eol(C);
  } else if (C->x == 0 && C->coloff > 0) {
    C->coloff--;
  } else {
    C->x--;
  }
}

void cursor_break_line(cursor_t* C) {
  vec2_t pos = cursor_position(C);

  undo_push(LINEBREAK, (vec2_t){0, pos.y+1}, *C, NULL);

  editor_break_line(C->editor, pos.x, pos.y);
  cursor_down(C);
  cursor_bol(C);
}

void cursor_move_word_forward(cursor_t* C) {
  char ch1, ch2;
  editor_t* E = C->editor;
  do {
    vec2_t pos = cursor_position(C);
    if (pos.y == E->row_size - 1 && pos.x >= editor_rowlen(E, pos.y))
      return;
    cursor_right(C);
    pos = cursor_position(C);
    ch1 = cursor_char(C);
    ch2 = editor_char_at(E, pos.x + 1, pos.y);
  } while(!(ch1 !=  ' ' && ch2 == ' '));
  cursor_right(C);
}

void cursor_move_word_backward(cursor_t* C) {
  char ch1, ch2;
  editor_t* E = C->editor;
  do {
    vec2_t pos = cursor_position(C);
    if (pos.y == 0 && pos.x == 0)
      break;

    cursor_left(C);
    ch1 = cursor_char(C);
    ch2 = editor_char_at(E, pos.x - 2, raw_y(C));
  } while(!(ch1 !=  ' ' && ch2 == ' '));
}

void cursor_remove_char(cursor_t* C) {
  vec2_t pos = cursor_position(C);

  if (pos.x == 0 && pos.y == 0) return;
  if (pos.x == 0 && pos.y > 0) {
    usize prlen = editor_rowlen(C->editor, pos.y-1);
    undo_push(LINEUP, (vec2_t){prlen, pos.y-1}, *C, NULL);
    editor_move_line_up(C->editor, pos.y);
    cursor_up(C);
    C->x = MIN(C->max_col, prlen);
    C->coloff = MAX(0, (i32)prlen - C->x);
    return;
  }

  vec2_t undo_pos = {pos.x - 1, pos.y};
  char strdata[2] = { editor_char_at(C->editor, undo_pos.x, undo_pos.y), '\0' };
  undo_push(BACKSPACE, undo_pos, *C, strdata);

  editor_delete_char_at(C->editor, pos);

  if (C->x == 0 && C->coloff > 0)
    C->coloff--;
  else
    C->x--;

  C->editor->dirty = true;
}

void cursor_insert_char(cursor_t* C, int ch) {
  vec2_t pos = cursor_position(C);
  editor_insert_char_at(C->editor, pos.x, pos.y, ch);

  undo_push(ADD, (vec2_t){pos.x + 1, pos.y}, *C, NULL);

  C->x++;
  if (C->x > C->max_col) {
    C->coloff++;
    C->x = C->max_col;
  }

  C->editor->dirty = true;
}

void cursor_insert_text(cursor_t* C, const char* text) {
  if (text == NULL) return;

  editor_t* E = C->editor;
  char* flt = NULL;
  usize len = strlen(text);
  usize start = 0;
  usize i = 0;
  usize n = 0;

  for (i = 0; i < len; i++) {
    if (text[i] == '\n') {
      vec2_t pos = cursor_position(C);
      if (n == 0) {
        flt = strdup(E->rows[pos.y].chars + pos.x);
        E->rows[pos.y].chars[pos.x] = '\0';
      }
      if(text[start] == '\n') start++;

      editor_insert_text(E, pos, text + start, i - start);
      editor_insert_row_at(E, pos.y + 1);

      cursor_down(C);
      cursor_bol(C);

      n++;
      start = i;
    }
  }

  if (start < i) {
    if(text[start] == '\n') start++;
    editor_insert_text(E, cursor_position(C), text + start, i - start);

    for (usize j = start; j < i; j++)
      cursor_right(C);

    if (flt != NULL) {
      editor_insert_text(E, cursor_position(C), flt, strlen(flt));
      free(flt);
    }
  }
}

void cursor_page_up(cursor_t* C) {
  for (i32 i = 0; i < C->max_row; i ++) {
    cursor_up(C);
  }
}

void cursor_page_down(cursor_t* C) {
  for (i32 i = 0; i < C->max_row; i ++) {
    cursor_down(C);
  }
}

void cursor_delete_forward(cursor_t* C) {
  vec2_t pos = cursor_position(C);
  usize len = editor_rowlen(C->editor, pos.y);
  line_t *lp = editor_text_between(C->editor, pos, (vec2_t){len, pos.y});
  undo_push(DELETE_FORWARD, pos, *C, lp->text);
  line_free(lp);

  editor_delete_forward(C->editor, pos.x, pos.y);
}

void cursor_delete_row(cursor_t* C) {
  i32 y = raw_y(C);
  bool ll = C->editor->row_size - (y + 1) == 0; //last line

  if (y == 0 && ll) { // editor must have at least one row
    C->editor->rows[0].chars[0] = '\0';
    cursor_bol(C);
    return;
  }

  char* strdata = strdup(C->editor->rows[y].chars);
  undo_push(LINEDELETE, (vec2_t){0, y}, *C, strdata);

  editor_delete_rows(C->editor, y, y);

  if (ll) {
    cursor_up(C);
    y--;
  }
  if (raw_x(C) > editor_rowlen(C->editor, y)) cursor_eol(C);
}

void cursor_eof(cursor_t* C) {
  editor_t* E = C->editor;
  if (E->row_size > C->max_row) {
    C->y = C->max_row - 1;
    C->rowoff = E->row_size - C->max_row;
  } else {
    C->y = E->row_size - 1;
    C->rowoff = 0;
  }
  cursor_eol(C);
}

void cursor_bof(cursor_t* C) {
  cursor_bol(C);
  C->y = 0;
  C->rowoff = 0;
}

cursor_t cursor_init(editor_t* E) {
  cursor_t C = {0};
  C.editor = E;
  C.region.cursor = (cursor_t*)malloc(sizeof(cursor_t));

  return C;
}
