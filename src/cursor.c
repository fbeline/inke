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

void cursor_set_max(u16 max_col, u16 max_row) {
  C.max_row = max_row;
  C.max_col = max_col;
}

char cursor_char(editor_t* E) {
  return E->rows[raw_y()].chars[raw_x()];
}

void cursor_bol(editor_t* E) {
  C.x = 0;
  C.coloff = 0;
}

void cursor_eol(editor_t* E) {
  i32 len = strlen(E->rows[raw_y()].chars);
  C.x = C.max_col;
  C.coloff = MAX(0, len - C.max_col);
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
    if (raw_y() >= E->row_size - 1 &&
        strlen(E->rows[raw_y()].chars) >= raw_x()) break;
    cursor_move(E, raw_y() + 1, raw_x());
  } while((ch = cursor_char(E)), ch  !=  ' ' && ch != '\0');
}

void cursor_move_word_backward(editor_t* E) {
  char ch;
  do {
    if (raw_y() == 0 && raw_x() == 0) break;
    cursor_move(E, raw_x(), raw_y());
  } while((ch = cursor_char(E)), ch  !=  ' ' && ch != '\0');
}

void cursor_remove_char(editor_t *E) {
  if (C.x == 0 && C.coloff == 0) {
    return editor_move_line_up(E, raw_y());
  }

  usize len = strlen(E->rows[E->cy].chars);
  memmove(E->rows[E->cy].chars + raw_x() - 1, 
          E->rows[E->cy].chars + raw_x(),
          len - raw_x() + 1);

  if (C.x == 0 && C.coloff > 0)
    C.coloff--;
  else
    C.x--;

  E->dirty = true;
}

void cursor_insert_char(editor_t* E, int c) {
  if (strlen(E->rows[raw_y()].chars) + 1 >= E->rows[raw_y()].size) {
    E->rows[raw_y()].size += 8;
    char* tmp = realloc(E->rows[raw_y()].chars, E->rows[raw_y()].size);
    if (tmp == NULL) return;
    E->rows[raw_y()].chars = tmp;
  }
  editor_insert_char_at(&E->rows[raw_y()], c, raw_x());

  C.x++;
  if (C.x > C.max_col) {
    C.coloff++;
    C.x = C.max_col;
  }

  E->dirty = true;
}

void editor_insert_text_at_cursor(editor_t* E, const char* text) {
  usize size = strlen(text);
  for (usize i = 0; i < size; i++) {
    if (text[i] == '\n') {
      editor_return(E);
      C.x = 0;
      C.coloff = 0;
    } else {
      cursor_insert_char(E, text[i]);
    }
  }
}

void cursor_down(editor_t* E) {
  if (raw_y() >= E->row_size - 1) return;

  if (C.y < C.max_row) {
    C.y++;
  } else {
    C.rowoff++;
  }

  usize row_len = strlen(E->rows[raw_y()].chars);
  if (raw_x() > row_len) {
    cursor_eol(E);
  }
}

void cursor_up(editor_t* E) {
  if (raw_y() <= 0) return;

  if (C.rowoff > 0) {
    C.rowoff--;
  } else {
    C.y--;
  }

  if (raw_x() > strlen(E->rows[raw_y()].chars)) {
    cursor_eol(E);
  }
}


void cursor_page_up(editor_t* E) {

}

void cursor_page_down(editor_t* E) {

}
