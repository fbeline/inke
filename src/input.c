#include "input.h"

#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include "editor.h"
#include "fs.h"
#include "utils.h"

void input_key_down(editor_t* E) {
  E->cy = MIN(E->cy+1, E->row_size-1);
  if (E->cy - E->rowoff >= E->screenrows) {
    E->rowoff = MIN(E->rowoff+1, E->row_size);
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
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_A)) {
    return editor_bol(E);
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E)) {
    return editor_eol(E);
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_K)) {
    return editor_delete_forward(E);
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
    char *buf = editor_rows_to_string(E->rows, E->row_size);
    FileWrite(E->filename, buf);
    free(buf);
  }
  if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_F)) {
    return editor_move_cursor_word_forward(E);
  }
  if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_B)) {
    return editor_move_cursor_word_backward(E);
  }
  if (IsKeyPressed(KEY_ENTER)) {
    return editor_return(E);
  }
  if (IsKeyPressed(KEY_RIGHT) || 
    IsKeyPressedRepeat(KEY_RIGHT) ||
    (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_F))) {
    editor_move_cursor(E, E->cx + E->coloff + 1, E->cy);
  }
  if (IsKeyPressed(KEY_DOWN) ||
    IsKeyPressedRepeat(KEY_DOWN) ||
    (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_N))) {
    input_key_down(E); 
  }
  if (IsKeyPressed(KEY_LEFT) ||
    IsKeyPressedRepeat(KEY_LEFT) ||
    (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_B))) {
    editor_move_cursor(E, E->cx + E->coloff - 1, E->cy);
  }
  if (IsKeyPressed(KEY_UP) ||
    IsKeyPressedRepeat(KEY_UP) ||
    (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_P))) {
    input_key_up(E);
  }
  if (IsKeyPressed(KEY_BACKSPACE)) {
    editor_remove_char_at_cursor(E);
  }

  int ch = GetCharPressed();
  if (IsCharBetween(ch, 32, 127))
    editor_insert_char_at_cursor(E, ch);
}
