#include "cursor.h"

#include <string.h>
#include <stdlib.h>
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

vec2_t cursor_position(void) {
  return (vec2_t) {raw_x(), raw_y()};
}

void cursor_set_max(u16 max_col, u16 max_row) {
  C.max_row = max_row;
  C.max_col = max_col;
}

char cursor_char(editor_t* E) {
  return editor_char_at(E, raw_x(), raw_y());
}

void cursor_bol(editor_t* E) {
  C.x = 0;
  C.coloff = 0;
}

void cursor_eol(editor_t* E) {
  i32 len = strlen(E->rows[raw_y()].chars);
  C.x = len,
  C.coloff = MAX(0, len - C.max_col);
}

void cursor_return(editor_t* E) {
  vec2_t pos = cursor_position();
  editor_break_line(E, pos.x, pos.y);
  cursor_move(E, 0, pos.y + 1);
}

void cursor_move(editor_t* E, i32 x, i32 y) {
  if (x < 0 && y == 0) return;

  i32 len = (i32)strlen(E->rows[y].chars);

  if (x < 0 && C.coloff > 0) {
    C.coloff--;
  } else if (x < 0 && C.coloff == 0) {
    y = MAX(0, y - 1);
    x = (i32)strlen(E->rows[y].chars);
    C.coloff = 0;
    cursor_move(E, x, y);
  } else if (x <= len && x >= C.max_col) {
    C.max_col = x - C.max_col;
    C.x = x - C.coloff;
  } else if (x > len) {
    C.coloff = 0;
    C.x = 0;
    y++;
  } else {
    C.x = x;
  }

  C.y = y;
}

void cursor_move_word_forward(editor_t* E) {
  char ch;
  do {
    vec2_t pos = cursor_position();
    if (pos.y >= E->row_size - 1 && strlen(E->rows[pos.y].chars) >= pos.x)
      break;
    cursor_move(E, pos.x + 1, pos.y);
  } while((ch = cursor_char(E)), ch  !=  ' ' && ch != '\0');
}

void cursor_move_word_backward(editor_t* E) {
  char ch;
  do {
    vec2_t pos = cursor_position();
    if (pos.y == 0 && pos.x == 0)
      break; 

    cursor_move(E, pos.x - 1, pos.y);
  } while((ch = cursor_char(E)), ch  !=  ' ' && ch != '\0');
}

void cursor_remove_char(editor_t *E) {
  vec2_t pos = cursor_position();
  if (pos.x == 0) {
    usize len = strlen(E->rows[pos.y - 1].chars);
    editor_move_line_up(E, pos.y);
    cursor_move(E, len, pos.y - 1);
    return;
  }

  usize len = strlen(E->rows[pos.y].chars);
  memmove(E->rows[pos.y].chars + pos.x - 1, 
          E->rows[pos.y].chars + pos.x,
          len - pos.x + 1);

  if (C.x == 0 && C.coloff > 0)
    C.coloff--;
  else
    C.x--;

  E->dirty = true;
}

void cursor_insert_char(editor_t* E, int c) {
  vec2_t pos = cursor_position();
  if (strlen(E->rows[pos.y].chars) + 1 >= E->rows[pos.y].size) {
    E->rows[pos.y].size += 8;
    char* tmp = realloc(E->rows[pos.y].chars, E->rows[pos.y].size);
    if (tmp == NULL) return;
    E->rows[pos.y].chars = tmp;
  }
  editor_insert_char_at(&E->rows[pos.y], c, pos.x);

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
      cursor_return(E);
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

void cursor_down(editor_t* E) {
  vec2_t pos = cursor_position();
  if (pos.y >= E->row_size - 1) return;

  if (C.y < C.max_row) {
    C.y++;
  } else {
    C.rowoff++;
  }

  usize row_len = strlen(E->rows[pos.y+1].chars);
  if (pos.x > row_len) {
    cursor_eol(E);
  }
}

void cursor_up(editor_t* E) {
  i32 y = raw_y();
  if (y <= 0) return;

  if (C.rowoff > 0) {
    C.rowoff--;
  } else {
    C.y--;
  }

  if (raw_x() > strlen(E->rows[y].chars)) {
    cursor_eol(E);
  }
}

void cursor_right(editor_t* E) {
  cursor_move(E, raw_x() + 1, raw_y());
}

void cursor_left(editor_t* E) {
  cursor_move(E, raw_x() - 1, raw_y());
}

