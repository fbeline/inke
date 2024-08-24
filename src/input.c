#include "input.h"

#include <stdlib.h>
#include <stdio.h>
#include <raylib.h>
#include "cursor.h"
#include "mode.h"
#include "fs.h"
#include "utils.h"

static void input_insert_handler(editor_t* E) { 
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_X)) {
    return mode_set_ctrl_x();
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_A)) {
    return cursor_bol();
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E)) {
    return cursor_eol(E);
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_K)) {
    return cursor_delete_forward(E);
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
  if (IsKeyDown(KEY_LEFT_ALT) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_COMMA)) {
    return cursor_bof();
  }
  if (IsKeyDown(KEY_LEFT_ALT) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_PERIOD)) {
    return cursor_eof(E);
  }
  if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_F)) {
    return cursor_move_word_forward(E);
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

  i32 ch = GetCharPressed();
  if (IN_RANGE(ch, 32, 95)) {
      cursor_insert_char(E, ch);
  }
}

static void input_command_chain_handler(editor_t* E) {
  if (IsKeyDown(KEY_LEFT_CONTROL)) {
    if (IsKeyPressed(KEY_C)) {
      if (E->dirty) {
        g_mode = COMMAND_CHAIN;
        mode_set_exit_save(E);
      } else {
        E->running = false;
      }
      return;
    }
    if (IsKeyPressed(KEY_S)) {
      char* buf = editor_rows_to_string(E->rows, E->row_size);
      FileWrite(E->filename, buf);
      free(buf);
      E->dirty = false;
      mode_cmd_clean();
    }
    if (IsKeyPressed(KEY_G)) {
      return mode_cmd_clean();
    }
  }
}

static void input_command_char_handler(editor_t* E) {
  i32 ch = GetCharPressed();
  if (IN_RANGE(ch, 32, 95)) {
    g_active_command.handler(E, ch);
  }
}

void input_keyboard_handler(editor_t* E) {
  if (g_mode & MODE_INSERT)
    input_insert_handler(E);
  else if (g_mode & COMMAND_CHAIN)
    input_command_chain_handler(E);
  else if (g_mode & COMMAND_SINGLE_CHAR)
    input_command_char_handler(E);
}
