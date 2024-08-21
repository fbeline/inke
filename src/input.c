#include "input.h"

#include <stdlib.h>
#include <raylib.h>
#include "cursor.h"
#include "fs.h"
#include "utils.h"

static void input_write_buffer(editor_t* E) {
  char *buf = editor_rows_to_string(E->rows, E->row_size);
  FileWrite(E->filename, buf);
  free(buf);
  E->dirty = false;
  E->new_file = false;
  return;
}

void input_keyboard_handler(editor_t* E, render_t* R) {
  if (IsKeyPressed(KEY_ESCAPE)) {
    if (E->dirty) {
      R->message_box = 1;
    } else {
      E->running = false;
    }
    return;
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_A)) {
    return cursor_bol(E);
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E)) {
    return cursor_eol(E);
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_K)) {
    return cursor_delete_forward(E);
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
    return input_write_buffer(E);
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Y)) {
    return cursor_insert_text(E, GetClipboardText());
  }
  if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_F)) {
    return cursor_move_word_forward(E);
  }
  if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_B)) {
    return cursor_move_word_backward(E);
  }
  if (IsKeyPressed(KEY_ENTER)) {
    return cursor_break_line(E);
  }
  if (IsKeyPressed(KEY_PAGE_DOWN)) {
    return cursor_page_down(E);
  }
  if (IsKeyPressed(KEY_PAGE_UP)) {
    return cursor_page_up(E);
  }
  if (IsKeyPressed(KEY_RIGHT) || 
    IsKeyPressedRepeat(KEY_RIGHT) ||
    (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_F))) {
    cursor_right(E);
  }
  if (IsKeyPressed(KEY_DOWN) ||
    IsKeyPressedRepeat(KEY_DOWN) ||
    (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_N))) {
    cursor_down(E); 
  }
  if (IsKeyPressed(KEY_LEFT) ||
    IsKeyPressedRepeat(KEY_LEFT) ||
    (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_B))) {
    cursor_left(E);
  }
  if (IsKeyPressed(KEY_UP) ||
    IsKeyPressedRepeat(KEY_UP) ||
    (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_P))) {
    cursor_up(E);
  }
  if (IsKeyPressed(KEY_BACKSPACE)) {
    cursor_remove_char(E);
  }

  int ch = GetCharPressed();
  if (IN_RANGE(ch, 32, 95)) {
    cursor_insert_char(E, ch);
  }
}
