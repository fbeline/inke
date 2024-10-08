#include "cursor.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ds.h"
#include "editor.h"
#include "globals.h"
#include "undo.h"
#include "utils.h"

static bool is_word_char(i32 c) {
   return isalnum(c) || c == '_';
}

static bool word_forward_stop(const char *line, u32 i) {
  if (line == NULL)
    return false;
  if (line[i+1] == '\0')
    return true;

  return is_word_char(line[i]) && !is_word_char(line[i+1]);
}

static bool word_backward_stop(const char *line, u32 i) {
  if (line == NULL)
    return false;
  if (i == 0)
    return true;

  return is_word_char(line[i]) && !is_word_char(line[i-1]);
}

static u32 raw_x(cursor_t* C) {
  return C->x + C->coloff;
}

static u32 raw_y(cursor_t* C) {
  return C->y + C->rowoff;
}

void cursor_set(buffer_t *dest, buffer_t* src) {
  dest->cursor->x = src->cursor->x;
  dest->cursor->y = src->cursor->y;
  dest->cursor->coloff = src->cursor->coloff;
  dest->cursor->rowoff = src->cursor->rowoff;

  dest->cursor->clp = dest->editor->lines;
  u32 max = dest->cursor->y + dest->cursor->rowoff;
  for (u32 i = 0; i < max; i++) {
    dest->cursor->clp = dest->cursor->clp->next;
  }
}

static void cursor_set_clp_as(buffer_t *B, line_t *lp, u32 x) {
  if (lp == NULL) return;

  cursor_t *C = B->cursor;
  C->clp = B->editor->lines;
  C->y = 0;
  C->x = 0;
  C->coloff = 0;
  C->rowoff = 0;

  while (C->clp != lp && C->clp != NULL) {
    cursor_down(B);
  }

  while(x-- > 0) {
    cursor_right(B);
  }
}

void cursor_region_text(buffer_t *B) {
  if (g_mode != MODE_VISUAL) return;

  editor_text_between(B->editor, mark_get(), g_clipbuf);

  g_mode = MODE_INSERT;
  clear_status_message();
}

void cursor_region_kill(buffer_t *B) {
  cursor_t *C = B->cursor;
  if (g_mode != MODE_VISUAL) return;

  mark_t mark = mark_get();
  editor_kill_between(B->editor, mark, g_clipbuf);

  cursor_set_clp_as(B, mark.start_lp, mark.start_offset);
  undo_push(CUT, B, g_clipbuf->buf);

  g_mode = MODE_INSERT;
  clear_status_message();
}

char cursor_char(buffer_t *B) {
  return editor_char_at(B->cursor->clp, raw_x(B->cursor));
}

void cursor_bol(buffer_t *B) {
  B->cursor->x = 0;
  B->cursor->coloff = 0;
}

void cursor_eol(buffer_t *B) {
  cursor_t *C = B->cursor;
  usize len = C->clp->ds->len;
  if (len > C->max_col) {
    C->x = C->max_col;
    C->coloff = len - C->max_col;
  } else {
    C->x = len;
    C->coloff = 0;
  }
}

void cursor_down(buffer_t *B) {
  cursor_t *C = B->cursor;
  if (C->clp->next == NULL) return;

  C->clp = C->clp->next;

  if (C->y < C->max_row) {
    C->y++;
  } else {
    C->rowoff++;
  }

  if (raw_x(C) > C->clp->ds->len) cursor_eol(B);
}

void cursor_up(buffer_t *B) {
  cursor_t *C = B->cursor;
  if (raw_y(C) <= 0 || C->clp->prev == NULL) return;
  C->clp = C->clp->prev;

  if (C->y == 0 && C->rowoff > 0) {
    C->rowoff--;
  } else {
    C->y--;
  }

  if (raw_x(C) > C->clp->ds->len) cursor_eol(B);
}

void cursor_right(buffer_t *B) {
  cursor_t *C = B->cursor;
  u32 x = raw_x(C);
  usize len = C->clp->ds->len;

  if (x < len && C->x == C->max_col) {
    C->coloff++;
  } else if (x >= len) {
    cursor_down(B);
    cursor_bol(B);
  } else {
    C->x++;
  }
}

void cursor_left(buffer_t *B) {
  u32 x = raw_x(B->cursor);
  u32 y = raw_y(B->cursor);
  if (x == 0 && y == 0) {
    return;
  } else if (x == 0 && y > 0) {
    cursor_up(B);
    cursor_eol(B);
  } else if (B->cursor->x == 0 && B->cursor->coloff > 0) {
    B->cursor->coloff--;
  } else {
    B->cursor->x--;
  }
}

void cursor_break_line(buffer_t *B) {
  u32 x = raw_x(B->cursor);

  editor_break_line(B->editor, B->cursor->clp, x);
  cursor_down(B);
  cursor_bol(B);

  undo_push(LINEBREAK, B, NULL);
}

void cursor_move_word_forward(buffer_t *B) {
  usize lastline = B->editor->row_size - 1;
  u32 x = raw_x(B->cursor);
  while(!word_forward_stop(B->cursor->clp->ds->buf, x)) {
    if (raw_y(B->cursor) == lastline && x >= B->cursor->clp->ds->len)
      return;
    cursor_right(B);
    x = raw_x(B->cursor);
  }
  cursor_right(B);
}

void cursor_move_word_backward(buffer_t *B) {
  cursor_left(B);
  u32 x = raw_x(B->cursor);
  while(!word_backward_stop(B->cursor->clp->ds->buf, x)) {
    if (raw_y(B->cursor) == 0 && x == 0)
      break;
    cursor_left(B);
    x = raw_x(B->cursor);
  }
}

void cursor_move_line_up(buffer_t *B) {
  cursor_t *C = B->cursor;
  if (C->clp->prev == NULL) return;

  line_t* lp;
  usize prlen = C->clp->prev->ds->len;
  if ((lp = editor_move_line_up(B->editor, C->clp)) == NULL) return;

  C->clp = lp;

  if (C->y == 0 && C->rowoff > 0) C->rowoff--;
  else C->y--;

  C->x = MIN(C->max_col, prlen);
  C->coloff = prlen > C->x ? prlen - C->x : 0;
}

void cursor_remove_char(buffer_t *B) {
  cursor_t *C = B->cursor;
  u32 x = raw_x(C);
  u32 y = raw_y(C);
  if (x == 0 && y == 0) return;
  if (x == 0 && y > 0) {
    cursor_move_line_up(B);
    undo_push(LINEUP, B, NULL);
    return;
  }

  char strdata[2] = { editor_char_at(C->clp, x - 1), '\0' };
  editor_delete_char_at(C->clp, x-1);

  if (C->x == 0 && C->coloff > 0)
    C->coloff--;
  else
    C->x--;

  B->editor->dirty = true;
  undo_push(BACKSPACE, B, strdata);
}

void cursor_insert_char(buffer_t *B, int ch) {
  editor_insert_char_at(B->editor, B->cursor->clp, raw_x(B->cursor), ch);
  cursor_right(B);

  undo_push(ADD, B, NULL);
}

void cursor_insert_text(buffer_t *B, const char* text) {
  for (usize i = 0, max = strlen(text); i < max; i++) {
    if (text[i] == '\n')
      cursor_break_line(B);
    else
      cursor_insert_char(B, text[i]);
  }
}

void cursor_page_up(buffer_t *B) {
  for (u16 i = 0; i < B->cursor->max_row; i ++) {
    cursor_up(B);
  }
}

void cursor_page_down(buffer_t *B) {
  for (u16 i = 0; i < B->cursor->max_row; i ++) {
    cursor_down(B);
  }
}

void cursor_delete_forward(buffer_t *B) {
  cursor_t *C = B->cursor;
  u32 x = raw_x(C);

  if (x == 0 || C->clp->ds->len == 0) {
    cursor_delete_row(B);
    return;
  }

  mark_t mark = {
    .start_lp = C->clp,
    .start_offset = x,
    .end_lp = C->clp,
    .end_offset = C->clp->ds->len
  };
  editor_text_between(B->editor, mark, g_clipbuf);
  undo_push(DELETE_FORWARD, B, g_clipbuf->buf);

  editor_delete_forward(C->clp, x);
}

void cursor_delete_row(buffer_t *B) {
  cursor_t *C = B->cursor;
  line_t *lp = NULL;
  g_clipbuf->buf[0] = '\0';
  g_clipbuf->len = 0;
  dscat(g_clipbuf, C->clp->ds->buf);
  undo_push(LINEDELETE, B, C->clp->ds->buf);

  if (C->clp->next != NULL) {
    lp = C->clp->next;
  } else if (C->clp->next == NULL && C->clp->prev != NULL) {
    lp = C->clp->prev;
    if (C->rowoff > 0) C->rowoff--;
    else C->y--;
  } else {
    lp = C->clp;
  }

  editor_delete_lines(B->editor, C->clp, 1);
  C->clp = lp;

  if (raw_x(C) > C->clp->ds->len) cursor_eol(B);
}

void cursor_eof(buffer_t *B) {
  editor_t *E = B->editor;
  cursor_t *C = B->cursor;
  if (E->row_size > C->max_row) {
    C->y = C->max_row - 1;
    C->rowoff = E->row_size - C->max_row;
  } else {
    C->y = E->row_size - 1;
    C->rowoff = 0;
  }
  while(C->clp->next != NULL)
    C->clp = C->clp->next;
  cursor_eol(B);
}

void cursor_bof(buffer_t *B) {
  cursor_bol(B);
  B->cursor->y = 0;
  B->cursor->rowoff = 0;
  while(B->cursor->clp->prev != NULL)
    B->cursor->clp = B->cursor->clp->prev;
}

void cursor_undo(buffer_t *B) {
  undo(B);
}

void cursor_paste(buffer_t *B) {
  cursor_insert_text(B, g_clipbuf->buf);
}

void cursor_update_window_size(buffer_t *B, u16 rows, u16 cols) {
  u32 x = raw_x(B->cursor);
  u32 y = raw_y(B->cursor);

  B->cursor->max_col = cols;
  B->cursor->max_row = rows;

  if (x > cols) {
    B->cursor->coloff = x - cols;
    B->cursor->x = cols;
  }
  if (y > rows) {
    B->cursor->rowoff = y - rows;
    B->cursor->y = rows;
  }
}

void cursor_goto(buffer_t *B, u32 x, u32 y) {
  cursor_bof(B);

  while (y-- > 1)
    cursor_down(B);

  x = MIN(x, B->cursor->clp->ds->len);
  while(x-- > 1)
    cursor_right(B);

}

void cursor_init(cursor_t *C) {
  C->coloff = 0;
  C->rowoff = 0;
  C->x = 0;
  C->y = 0;
}

void cursor_free(cursor_t *C) {
  if (C == NULL) return;

  free(C);
}
