#include "input.h"

#include <stdlib.h>
#include <stdio.h>
#include <raylib.h>
#include "cursor.h"
#include "mode.h"
#include "utils.h"

void input_keyboard_handler(editor_t* E, render_t* R) {
  if (IsKeyPressed(KEY_ESCAPE)) {
    if (E->dirty) {
      g_mode = MODE_COMMAND;
      mode_set_exit_save(E);
    } else {
      E->running = false;
    }
    return;
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_G)) {
    g_mode = MODE_INSERT;
    g_active_command = (command_t){0};
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
    /* return input_write_buffer(E); */
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Y)) {
    return cursor_insert_text(E, GetClipboardText());
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_SPACE)) {
    return cursor_region_start();
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_BACKSPACE)) {
    return cursor_delete_row(E);
  }
  if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_W)) {
    char* txt = cursor_region_text(E);
    if (txt != NULL) {
      SetClipboardText(txt);
      free(txt);
    }

    cursor_clear_region();
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
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_W)) {
    char* txt = cursor_region_kill(E);
    if (txt != NULL) {
      SetClipboardText(txt);
      free(txt);
    }
    cursor_clear_region();
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
    if (g_mode & MODE_INSERT)
      cursor_insert_char(E, ch);
    else if (g_mode & MODE_COMMAND)
      g_active_command.handler(E, ch);
  }
}
