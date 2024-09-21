#include "cursor.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "editor.h"
#include "globals.h"
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

  dest->clp = dest->editor->lines;
  usize max = dest->y + dest->rowoff;
  for (usize i = 0; i < max; i++) {
    dest->clp = dest->clp->next;
  }
}

vec2_t cursor_position(cursor_t* cursor) {
  return (vec2_t) {raw_x(cursor), raw_y(cursor)};
}

void cursor_region_start(cursor_t* C) {
  if (g_mode != MODE_VISUAL) {
    g_mode = MODE_VISUAL;
    C->region.lp = C->clp;
    C->region.offset = raw_x(C);
    C->region.size = 0;
    memcpy(C->region.cursor, C, sizeof(cursor_t));
    set_status_message("visual mode");
  } else {
    cursor_clear_region(C);
  }
}

char* cursor_region_text(cursor_t* C) {
  if (g_mode != MODE_VISUAL) return NULL;

  return editor_text_between(C->region.lp, C->region.offset, C->region.size);
}

char* cursor_region_kill(cursor_t* C) {
  if (g_mode != MODE_VISUAL) return NULL;

  char *strdata = editor_kill_between(C->editor, C->region.lp, C->region.offset, C->region.size);
  cursor_set(C, C->region.cursor);
  undo_push(CUT, *C, strdata);

  return strdata;
}

void cursor_clear_region(cursor_t* C) {
  g_mode = MODE_INSERT;
  C->region.size = 0;
  C->region.lp = NULL;
  set_status_message("");
}

void cursor_set_max(cursor_t* C, u16 max_col, u16 max_row) {
  C->max_row = max_row;
  C->max_col = max_col;
}

char cursor_char(cursor_t* C) {
  return editor_char_at(C->clp, raw_x(C));
}

void __cursor_bol(cursor_t* C, bool region) {
  if (region) {
    i32 x = raw_x(C);
    C->region.size -= x;
  }

  C->x = 0;
  C->coloff = 0;
}

void cursor_bol(cursor_t* C) {
  __cursor_bol(C, g_mode == MODE_VISUAL);
}

static void __cursor_eol(cursor_t* C, bool region) {
  i32 len = C->clp->size;
  i32 oldx = raw_x(C);
  if (len > C->max_col) {
    C->x = C->max_col;
    C->coloff = len - C->max_col;
  } else {
    C->x = len;
    C->coloff = 0;
  }

  if (region)
    C->region.size += raw_x(C) - oldx;
}

void cursor_eol(cursor_t* C) {
  __cursor_eol(C, g_mode == MODE_VISUAL);
}

static void __cursor_down(cursor_t* C, bool region) {
  vec2_t pos = cursor_position(C);
  if (C->clp->next == NULL) return;
  line_t* p_lp = C->clp;
  C->clp = C->clp->next;

  if (C->y < C->max_row) {
    C->y++;
  } else {
    C->rowoff++;
  }

  if (pos.x > C->clp->size)
    __cursor_eol(C, false);

  if (region)
    C->region.size += raw_x(C) + (p_lp->size - pos.x);
}

void cursor_down(cursor_t* C) {
  __cursor_down(C, g_mode == MODE_VISUAL);
}

static void __cursor_up(cursor_t* C, bool region) {
  i32 oldx = raw_x(C);

  if (raw_y(C) <= 0 || C->clp->prev == NULL) return;
  line_t* n_lp = C->clp;
  C->clp = C->clp->prev;

  if (C->y == 0 && C->rowoff > 0) {
    C->rowoff--;
  } else {
    C->y--;
  }

  if (raw_x(C) > C->clp->size)
    __cursor_eol(C, false);

  if (region)
    C->region.size = C->region.size - oldx - (C->clp->size - raw_x(C));
}

void cursor_up(cursor_t* C) {
  __cursor_up(C, g_mode == MODE_VISUAL);
}

void cursor_right(cursor_t* C) {
  vec2_t pos = cursor_position(C);
  usize len = C->clp->size;

  if (pos.x < len && C->x == C->max_col) {
    C->coloff++;
  } else if (pos.x >= len) {
    __cursor_down(C, false);
    __cursor_bol(C, false);
    C->region.size--; // do not increase last (blank) x position
  } else {
    C->x++;
  }

  C->region.size++;
}

void cursor_left(cursor_t* C) {
  vec2_t pos = cursor_position(C);
  if (pos.x == 0 && pos.y == 0) {
    return;
  } else if (pos.x == 0 && pos.y > 0) {
    __cursor_up(C, false);
    __cursor_eol(C, false);
    C->region.size++; // do not decrement first x position
  } else if (C->x == 0 && C->coloff > 0) {
    C->coloff--;
  } else {
    C->x--;
  }

  C->region.size--;
}

void cursor_break_line(cursor_t* C) {
  vec2_t pos = cursor_position(C);

  editor_break_line(C->editor, C->clp, pos.x);
  cursor_down(C);
  cursor_bol(C);

  undo_push(LINEBREAK, *C, NULL);
}

void cursor_move_word_forward(cursor_t* C) {
  char ch1, ch2;
  editor_t* E = C->editor;
  do {
    vec2_t pos = cursor_position(C);
    if (pos.y == E->row_size - 1 && pos.x >= C->clp->size)
      return;
    cursor_right(C);
    pos = cursor_position(C);
    ch1 = cursor_char(C);
    ch2 = editor_char_at(C->clp, pos.x + 1);
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
    ch2 = editor_char_at(C->clp, pos.x - 2);
  } while(!(ch1 !=  ' ' && ch2 == ' '));
}

void cursor_move_line_up(cursor_t *C) {
  if (C->clp->prev == NULL) return;

  line_t* lp;
  usize prlen = C->clp->prev->size;
  if ((lp = editor_move_line_up(C->editor, C->clp)) == NULL) return;

  C->clp = lp;

  if (C->y == 0 && C->rowoff > 0) C->rowoff--;
  else C->y--;

  C->x = MIN(C->max_col, prlen);
  C->coloff = MAX(0, (i32)prlen - C->x);
}

void cursor_remove_char(cursor_t* C) {
  vec2_t pos = cursor_position(C);
  char strdata[2] = { editor_char_at(C->clp, pos.x - 1), '\0' };

  if (pos.x == 0 && pos.y == 0) return;
  if (pos.x == 0 && pos.y > 0) {
    cursor_move_line_up(C);
    undo_push(LINEUP, *C, NULL);
    return;
  }

  editor_delete_char_at(C->clp, pos.x-1);

  if (C->x == 0 && C->coloff > 0)
    C->coloff--;
  else
    C->x--;

  C->editor->dirty = true;
  undo_push(BACKSPACE, *C, strdata);
}

void cursor_insert_char(cursor_t* C, int ch) {
  editor_insert_char_at(C->editor, C->clp, raw_x(C), ch);
  cursor_right(C);

  undo_push(ADD, *C, NULL);
}

void cursor_insert_text(cursor_t* C, const char* text) {
  editor_insert_text(C->editor, C->clp, raw_x(C), text);
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
  i32 x = raw_x(C);

  if (x == 0 || C->clp->size == 0)
    return cursor_delete_row(C);

  char *text = editor_text_between(C->clp, x, C->clp->size - x);
  undo_push(DELETE_FORWARD, *C, text);
  free(text);

  editor_delete_forward(C->clp, x);
}

void cursor_delete_row(cursor_t* C) {
  i32 y = raw_y(C);
  bool ll = C->editor->row_size - (y + 1) == 0; //last line

  char* strdata = strdup(C->clp->text);
  undo_push(LINEDELETE, *C, strdata);

  line_t *lp = C->clp->next != NULL ? C->clp->next : C->clp;
  editor_delete_lines(C->editor, C->clp, 1);
  C->clp = lp;

  if (ll) {
    cursor_up(C);
    y--;
  }
  if (raw_x(C) > C->clp->size) cursor_eol(C);
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
  while(C->clp->next != NULL)
    C->clp = C->clp->next;
  cursor_eol(C);
}

void cursor_bof(cursor_t* C) {
  cursor_bol(C);
  C->y = 0;
  C->rowoff = 0;
  while(C->clp->prev != NULL)
    C->clp = C->clp->prev;
}


void cursor_undo(cursor_t* C) {
  undo(C);
}

cursor_t cursor_init(editor_t* E) {
  cursor_t C = {0};
  C.editor = E;
  C.clp = E->lines;
  C.coloff = 0;
  C.rowoff = 0;
  C.x = 0;
  C.y = 0;
  C.region.cursor = (cursor_t*)malloc(sizeof(cursor_t));

  return C;
}
