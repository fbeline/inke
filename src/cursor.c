#include "cursor.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ds.h"
#include "editor.h"
#include "globals.h"
#include "undo.h"
#include "utils.h"

static usize raw_x(cursor_t* C) {
  return C->x + C->coloff;
}

static usize raw_y(cursor_t* C) {
  return C->y + C->rowoff;
}

void cursor_set(cursor_t* dest, cursor_t* src) {
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

static void cursor_set_clp_as(cursor_t *C, line_t *lp, usize x) {
  if (lp == NULL || x < 0) return;

  C->clp = C->editor->lines;
  C->y = 0;
  C->x = 0;
  C->coloff = 0;
  C->rowoff = 0;

  while (C->clp != lp && C->clp != NULL) {
    cursor_down(C);
  }

  while(x-- > 0) {
    cursor_right(C);
  }
}

vec2_t cursor_position(cursor_t* cursor) {
  return (vec2_t) {raw_x(cursor), raw_y(cursor)};
}

void cursor_region_text(cursor_t* C) {
  if (g_mode != MODE_VISUAL) return;

  editor_text_between(C->editor, mark_get(), g_clipbuf);

  g_mode = MODE_INSERT;
  clear_status_message();
}

void cursor_region_kill(cursor_t* C) {
  if (g_mode != MODE_VISUAL) return;

  mark_t mark = mark_get();
  editor_kill_between(C->editor, mark, g_clipbuf);

  cursor_set_clp_as(C, mark.start_lp, mark.start_offset);
  undo_push(CUT, *C, g_clipbuf->buf);

  g_mode = MODE_INSERT;
  clear_status_message();
}

char cursor_char(cursor_t* C) {
  return editor_char_at(C->clp, raw_x(C));
}

void cursor_bol(cursor_t* C) {
  C->x = 0;
  C->coloff = 0;
}

void cursor_eol(cursor_t* C) {
  usize len = C->clp->size;
  if (len > C->max_col) {
    C->x = C->max_col;
    C->coloff = len - C->max_col;
  } else {
    C->x = len;
    C->coloff = 0;
  }
}

void cursor_down(cursor_t* C) {
  if (C->clp->next == NULL) return;

  C->clp = C->clp->next;

  if (C->y < C->max_row) {
    C->y++;
  } else {
    C->rowoff++;
  }

  if (raw_x(C) > C->clp->size) cursor_eol(C);
}

void cursor_up(cursor_t* C) {
  if (raw_y(C) <= 0 || C->clp->prev == NULL) return;
  C->clp = C->clp->prev;

  if (C->y == 0 && C->rowoff > 0) {
    C->rowoff--;
  } else {
    C->y--;
  }

  if (raw_x(C) > C->clp->size) cursor_eol(C);
}

void cursor_right(cursor_t* C) {
  vec2_t pos = cursor_position(C);
  usize len = C->clp->size;

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
  C->coloff = prlen > C->x ? prlen - C->x : 0;;
}

void cursor_remove_char(cursor_t* C) {
  vec2_t pos = cursor_position(C);
  if (pos.x == 0 && pos.y == 0) return;
  if (pos.x == 0 && pos.y > 0) {
    cursor_move_line_up(C);
    undo_push(LINEUP, *C, NULL);
    return;
  }

  char strdata[2] = { editor_char_at(C->clp, pos.x - 1), '\0' };
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
  for (usize i = 0, max = strlen(text); i < max; i++) {
    if (text[i] == '\n')
      cursor_break_line(C);
    else
      cursor_insert_char(C, text[i]);
  }
}

void cursor_page_up(cursor_t* C) {
  for (usize i = 0; i < C->max_row; i ++) {
    cursor_up(C);
  }
}

void cursor_page_down(cursor_t* C) {
  for (usize i = 0; i < C->max_row; i ++) {
    cursor_down(C);
  }
}

void cursor_delete_forward(cursor_t* C) {
  i32 x = raw_x(C);

  if (x == 0 || C->clp->size == 0) {
    cursor_delete_row(C);
    return;
  }

  mark_t mark = {
    .start_lp = C->clp,
    .start_offset = x,
    .end_lp = C->clp,
    .end_offset = C->clp->size
  };
  editor_text_between(C->editor, mark, g_clipbuf);
  undo_push(DELETE_FORWARD, *C, g_clipbuf->buf);

  editor_delete_forward(C->clp, x);
}

void cursor_delete_row(cursor_t* C) {
  line_t *lp = NULL;
  undo_push(LINEDELETE, *C, C->clp->text);

  if (C->clp->next != NULL) {
    lp = C->clp->next;
  } else if (C->clp->next == NULL && C->clp->prev != NULL) {
    lp = C->clp->prev;
    if (C->rowoff > 0) C->rowoff--;
    else C->x--;
  } else {
    lp = C->clp;
  }

  editor_delete_lines(C->editor, C->clp, 1);
  C->clp = lp;

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

void cursor_paste(cursor_t *C) {
  cursor_insert_text(C, g_clipbuf->buf);
}

cursor_t cursor_init(editor_t* E) {
  cursor_t C = {0};
  C.editor = E;
  C.clp = E->lines;
  C.coloff = 0;
  C.rowoff = 0;
  C.x = 0;
  C.y = 0;

  return C;
}

void cursor_update_window_size(cursor_t *C, u16 rows, u16 cols) {
  u32 x = raw_x(C);
  u32 y = raw_y(C);

  C->max_col = cols;
  C->max_row = rows;

  if (x > cols) {
    C->coloff = x - cols;
    C->x = cols;
  }
  if (y > rows) {
    C->rowoff = y - rows;
    C->y = rows;
  }
}
