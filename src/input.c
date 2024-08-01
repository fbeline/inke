#include "input.h"

#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include "utils.h"

void input_return(editor_t* E) {
  if (!editor_insert_row_at(E, E->cy + 1)) return;

  E->rows[E->cy + 1].size = strlen(E->rows[E->cy].chars) - E->cx + 2;
  E->rows[E->cy + 1].chars = malloc(E->rows[E->cy + 1].size);
  memcpy(E->rows[E->cy + 1].chars,
         E->rows[E->cy].chars + E->cx,
         strlen(E->rows[E->cy].chars) - E->cx + 1);

  E->rows[E->cy].chars[E->cx] = '\0';

  E->cy++;
  E->cx = 0;
}

void input_key_down(editor_t* E) {
  E->cy = MIN(E->cy+1, E->rowslen-1);
  if (E->cy - E->rowoff >= E->screenrows) {
    E->rowoff = MIN(E->rowoff+1, E->rowslen);
  }

  usize row_len = strlen(E->rows[E->cy].chars);
  if (E->cx > row_len) {
    editor_eol(E);
  }
}

void input_key_up(editor_t* E) {
  E->cy = MAX(E->cy-1, 0);
  if (E->cy - E->rowoff <= 0)
    E->rowoff = MAX(E->rowoff-1, 0);

  if (E->cx > strlen(E->rows[E->cy].chars)) {
    editor_eol(E);
  }
}

void input_mousewheel_handler(editor_t* E) {
  float mouseWheelMove = GetMouseWheelMove();
  if (mouseWheelMove > 0) {
    E->rowoff = MAX(E->rowoff - 1, 0);
  }
  else if (mouseWheelMove < 0) {
    E->rowoff = MIN(E->rowoff + 1, E->rowslen - 1);  // Mouse wheel down
  }
}

void input_keyboard_handler(editor_t* E) {
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_A)) {
    editor_bol(E);
    return;
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_E)) {
    editor_eol(E);
    return;
  }
  if (IsKeyPressed(KEY_ENTER)) {
    input_return(E);
    return;
  }
  if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) {
    E->cx++;
    if (E->cx + E->coloff > MAX_COL) {
      E->cx = MAX_COL;
      E->coloff++;
    }
    if (E->cx + E->coloff > strlen(E->rows[E->cy].chars)) {
      E->coloff = 0;
      E->cx = 0;
      E->cy++;
    }
  }
  if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN)) {
    input_key_down(E); 
  }
  if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) {
    E->cx--;
  
    if (E->cx < 0) {
      if (E->cy == 0) {
        E->cx = 0;
        return;
      }
      int rowlen = strlen(E->rows[MAX(0, E->cy-1)].chars);
      if (E->coloff == 0) {
        E->cx = MIN(rowlen, MAX_COL);
        E->cy = MAX(E->cy - 1, 0);
        if (rowlen > MAX_COL) E->coloff = rowlen - MAX_COL;
      } else {
        E->coloff--;
        E->cx = 0;
      }
    } 
  }
  if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)) {
    input_key_up(E);
  }
  if (IsKeyPressed(KEY_BACKSPACE)) {
    editor_remove_char_at_cursor(E);
  }
  if (IsKeyPressed(KEY_F10)) {
    char *buf = editor_rows_to_string(E->rows, E->rowslen);
    FileWrite(E->filename, buf);
    free(buf);
  }

  int ch = GetCharPressed();
  if (IsCharBetween(ch, 32, 127))
    editor_insert_char_at_cursor(E, ch);
}
