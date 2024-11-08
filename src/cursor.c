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

u32 raw_x(cursor_t* C) {
  return C->x + C->coloff;
}

u32 raw_y(cursor_t* C) {
  return C->y + C->rowoff;
}

void cursor_set(buffer_t *dest, cursor_t* src) {
  dest->cursor = *src;
  dest->lp = dest->editor.lines;

  u32 max = src->y + src->rowoff;
  for (u32 i = 0; i < max; i++) {
    dest->lp = dest->lp->next;
  }
}

static void cursor_set_lp_as(buffer_t *B, line_t *lp, u32 x) {
  if (lp == NULL) return;

  cursor_t *C = &B->cursor;
  B->lp = B->editor.lines;
  C->y = 0;
  C->x = 0;
  C->coloff = 0;
  C->rowoff = 0;

  while (B->lp != lp && B->lp != NULL) {
    cursor_down(B);
  }

  while(x-- > 0) {
    cursor_right(B);
  }
}

void cursor_region_text(buffer_t *B) {
  if (!(g_flags & MVISUAL)) return;

  editor_text_between(&B->editor, mark_get(), g_clipbuf);

  g_flags &= ~MVISUAL;
  g_flags |= MINSERT;
  set_status_message("");
}

void cursor_region_kill(buffer_t *B) {
  if (!(g_flags & MVISUAL)) return;

  mark_t mark = mark_get();
  editor_kill_between(&B->editor, mark, g_clipbuf);

  B->lp = mark.start_lp;
  B->cursor = mark.start_cursor;

  undo_push(CUT, B, g_clipbuf->buf);

  g_flags &= ~MVISUAL;
  g_flags |= MINSERT;
  set_status_message("");

  B->dirty++;
}

char cursor_char(buffer_t *B) {
  return editor_char_at(B->lp, raw_x(&B->cursor));
}

void cursor_bol(buffer_t *B) {
  B->cursor.x = 0;
  B->cursor.coloff = 0;
}

void cursor_eol(buffer_t *B) {
  cursor_t *C = &B->cursor;
  usize len = B->lp->ds->len;
  if (len > g_window.ncol) {
    C->x = g_window.ncol;
    C->coloff = len - g_window.ncol;
  } else {
    C->x = len;
    C->coloff = 0;
  }
}

void cursor_down(buffer_t *B) {
  cursor_t *C = &B->cursor;
  if (B->lp->next == NULL) return;

  B->lp = B->lp->next;

  if (C->y < g_window.nrow) {
    C->y++;
  } else {
    C->rowoff++;
  }

  if (raw_x(C) > B->lp->ds->len) cursor_eol(B);
}

void cursor_up(buffer_t *B) {
  cursor_t *C = &B->cursor;
  if (raw_y(C) <= 0 || B->lp->prev == NULL) return;
  B->lp = B->lp->prev;

  if (C->y == 0 && C->rowoff > 0) {
    C->rowoff--;
  } else {
    C->y--;
  }

  if (raw_x(C) > B->lp->ds->len) cursor_eol(B);
}

void cursor_right(buffer_t *B) {
  cursor_t *C = &B->cursor;
  u32 x = raw_x(C);
  usize len = B->lp->ds->len;

  if (x < len && C->x == g_window.ncol) {
    C->coloff++;
  } else if (x >= len) {
    cursor_down(B);
    cursor_bol(B);
  } else {
    C->x++;
  }
}

void cursor_left(buffer_t *B) {
  u32 x = raw_x(&B->cursor);
  u32 y = raw_y(&B->cursor);
  if (x == 0 && y == 0) {
    return;
  } else if (x == 0 && y > 0) {
    cursor_up(B);
    cursor_eol(B);
  } else if (B->cursor.x == 0 && B->cursor.coloff > 0) {
    B->cursor.coloff--;
  } else {
    B->cursor.x--;
  }
}

void cursor_break_line(buffer_t *B) {
  u32 x = raw_x(&B->cursor);

  editor_break_line(&B->editor, B->lp, x);
  cursor_down(B);
  cursor_bol(B);

  undo_push(LINEBREAK, B, NULL);

  B->dirty++;
}

void cursor_move_word_forward(buffer_t *B) {
  usize lastline = B->editor.nlines - 1;
  u32 x = raw_x(&B->cursor);
  while(!word_forward_stop(B->lp->ds->buf, x)) {
    if (raw_y(&B->cursor) == lastline && x >= B->lp->ds->len)
      return;
    cursor_right(B);
    x = raw_x(&B->cursor);
  }
  cursor_right(B);
}

void cursor_move_word_backward(buffer_t *B) {
  cursor_left(B);
  u32 x = raw_x(&B->cursor);
  while(!word_backward_stop(B->lp->ds->buf, x)) {
    if (raw_y(&B->cursor) == 0 && x == 0)
      break;
    cursor_left(B);
    x = raw_x(&B->cursor);
  }
}

void cursor_move_line_up(buffer_t *B) {
  cursor_t *C = &B->cursor;
  if (B->lp->prev == NULL) return;

  line_t* lp;
  usize prlen = B->lp->prev->ds->len;
  if ((lp = editor_move_line_up(&B->editor, B->lp)) == NULL) return;

  B->lp = lp;

  if (C->y == 0 && C->rowoff > 0) C->rowoff--;
  else C->y--;

  C->x = MIN(g_window.ncol, prlen);
  C->coloff = prlen > C->x ? prlen - C->x : 0;

  B->dirty++;
}

void cursor_remove_char(buffer_t *B) {
  cursor_t *C = &B->cursor;
  u32 x = raw_x(C);
  u32 y = raw_y(C);
  if (x == 0 && y == 0) return;
  if (x == 0 && y > 0) {
    cursor_move_line_up(B);
    undo_push(LINEUP, B, NULL);
    return;
  }

  char strdata[2] = { editor_char_at(B->lp, x - 1), '\0' };
  editor_delete_char_at(B->lp, x-1);

  if (C->x == 0 && C->coloff > 0)
    C->coloff--;
  else
    C->x--;

  undo_push(BACKSPACE, B, strdata);

  B->dirty++;
}

void cursor_insert_char(buffer_t *B, int ch) {
  editor_insert_char_at(&B->editor, B->lp, raw_x(&B->cursor), ch);
  cursor_right(B);
  B->dirty++;

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
  for (u32 i = 0; i < g_window.nrow; i ++) {
    cursor_up(B);
  }
}

void cursor_page_down(buffer_t *B) {
  for (u32 i = 0; i < g_window.nrow; i ++) {
    cursor_down(B);
  }
}

void cursor_delete_forward(buffer_t *B) {
  cursor_t *C = &B->cursor;
  u32 x = raw_x(C);

  if (x == 0 || B->lp->ds->len == 0) {
    cursor_delete_row(B);
    return;
  }

  mark_t mark = {
    .start_lp = B->lp,
    .start_offset = x,
    .end_lp = B->lp,
    .end_offset = B->lp->ds->len
  };
  editor_text_between(&B->editor, mark, g_clipbuf);
  undo_push(DELETE_FORWARD, B, g_clipbuf->buf);

  editor_delete_forward(B->lp, x);

  B->dirty++;
}

void cursor_delete_row(buffer_t *B) {
  cursor_t *C = &B->cursor;
  line_t *lp = NULL;
  g_clipbuf->buf[0] = '\0';
  g_clipbuf->len = 0;
  dscat(g_clipbuf, B->lp->ds->buf);
  undo_push(LINEDELETE, B, B->lp->ds->buf);

  if (B->lp->next != NULL) {
    lp = B->lp->next;
  } else if (B->lp->next == NULL && B->lp->prev != NULL) {
    lp = B->lp->prev;
    if (C->rowoff > 0) C->rowoff--;
    else C->y--;
  } else {
    lp = B->lp;
  }

  editor_delete_lines(&B->editor, B->lp, 1);
  B->lp = lp;

  if (raw_x(C) > B->lp->ds->len) cursor_eol(B);

  B->dirty++;
}

void cursor_eof(buffer_t *B) {
  editor_t *E = &B->editor;
  cursor_t *C = &B->cursor;
  if (E->nlines > g_window.nrow) {
    C->y = g_window.nrow - 1;
    C->rowoff = E->nlines - g_window.nrow;
  } else {
    C->y = E->nlines - 1;
    C->rowoff = 0;
  }
  while(B->lp->next != NULL)
    B->lp = B->lp->next;
  cursor_eol(B);
}

void cursor_bof(buffer_t *B) {
  cursor_bol(B);
  B->cursor.y = 0;
  B->cursor.rowoff = 0;
  while(B->lp->prev != NULL)
    B->lp = B->lp->prev;
}

void cursor_undo(buffer_t *B) {
  undo(B);
}

void cursor_paste(buffer_t *B) {
  cursor_insert_text(B, g_clipbuf->buf);
}

void cursor_update_window_size(buffer_t *B, u16 rows, u16 cols) {
  u32 x = raw_x(&B->cursor);
  u32 y = raw_y(&B->cursor);

  g_window.ncol = cols;
  g_window.nrow = rows;

  if (x > cols) {
    B->cursor.coloff = x - cols;
    B->cursor.x = cols;
  }
  if (y > rows) {
    B->cursor.rowoff = y - rows;
    B->cursor.y = rows;
  }
}

void cursor_goto(buffer_t *B, u32 x, u32 y) {
  cursor_bof(B);

  while (y-- > 1)
    cursor_down(B);

  x = MIN(x, B->lp->ds->len);
  while(x-- > 1)
    cursor_right(B);

}

void cursor_init(cursor_t *C) {
  C->coloff = 0;
  C->rowoff = 0;
  C->x = 0;
  C->y = 0;
}

void cursor_move_region_forward(buffer_t *B) {
  mark_t mark = mark_get();
  line_t *lp = mark.start_lp;

  while(lp != NULL) {
    for (u8 i = 0; i < TAB_STOP; i++) {
      editor_insert_char_at(&B->editor, lp, 0, ' ');
    }
    lp = lp != mark.end_lp ? lp->next : NULL;
  }
}

void cursor_move_region_backward(buffer_t *B) {
  mark_t mark = mark_get();
  line_t *lp = mark.start_lp;

  while (lp != NULL) {
    for (u8 i = 0; i < TAB_STOP; i++) {
      if (lp->ds->buf[0] != ' ') break;

      editor_delete_char_at(lp, 0);
      if (lp == g_mark.start_lp && g_mark.start_offset > 0)
        g_mark.start_offset--;
      if (lp == g_mark.end_lp && g_mark.end_offset > 0)
        g_mark.end_offset--;
    }
    lp = lp != mark.end_lp ? lp->next : NULL;
  }

  if (raw_x(&B->cursor) > B->lp->ds->len)
    cursor_eol(B);
}

void cursor_free(cursor_t *C) {
  if (C == NULL) return;

  free(C);
}

void cursor_move_paragraph_forward(buffer_t *B) {
  if (raw_y(&B->cursor) == B->editor.nlines) return;

  cursor_bol(B);
  do {
    cursor_down(B);
  } while(B->lp->ds->len > 0 && raw_y(&B->cursor) < B->editor.nlines - 1);
}

void cursor_move_paragraph_backward(buffer_t *B) {
  if (raw_y(&B->cursor) == 0) return;

  cursor_bol(B);
  do {
    cursor_up(B);
  } while(B->lp->ds->len > 0 && raw_y(&B->cursor) > 0);
}

void cursor_recenter(buffer_t *B) {
  i64 y = (i64)raw_y(&B->cursor);
  i64 target_y = g_window.nrow / 2;

  if (y <= target_y || B->cursor.y == target_y) return;

  i64 offset = target_y - B->cursor.y;
  if (offset <= B->cursor.rowoff) {
    B->cursor.rowoff -= offset;
    B->cursor.y = target_y;
  }
}
