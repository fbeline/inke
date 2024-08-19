#include "input.h"

#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include "cursor.h"
#include "fs.h"
#include "utils.h"

static void input_key_down(editor_t* E) {
  cursor_down(E);
}

static void input_key_up(editor_t* E) {
  cursor_up(E); 
}

static void input_page_down(editor_t* E) {
  E->cy = MIN(E->cy + MAX_ROW, (i64)E->row_size-1);
  E->rowoff = E->cy;
}

static void input_page_up(editor_t* E) {
  E->cy = MAX(E->cy - MAX_ROW, 0);
  E->rowoff = MAX(E->cy - MAX_ROW + 1, 0);
}

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
    return editor_bol(E);
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E)) {
    return editor_eol(E);
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_K)) {
    return editor_delete_forward(E);
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
    return input_write_buffer(E);
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Y)) {
    return editor_insert_text_at_cursor(E, GetClipboardText());
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
  if (IsKeyPressed(KEY_PAGE_DOWN)) {
    return input_page_down(E);
  }
  if (IsKeyPressed(KEY_PAGE_UP)) {
    return input_page_up(E);
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
  if (IN_RANGE(ch, 32, 95)) {
    editor_insert_char_at_cursor(E, ch);
  }
}
