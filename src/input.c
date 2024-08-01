#include "input.h"

#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include "utils.h"

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
    editor_return(E);
    return;
  }
  if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) {
    editor_move_cursor_right(E);
  }
  if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN)) {
    input_key_down(E); 
  }
  if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) {
    editor_move_cursor_left(E); 
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
