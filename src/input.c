#include "input.h"

#include <stdlib.h>
#include <stdio.h>
#include <raylib.h>
#include <string.h>
#include "undo.h"
#include "cursor.h"
#include "mode.h"
#include "io.h"
#include "utils.h"

static bool kpr(int k) {
  return IsKeyPressed(k) || IsKeyPressedRepeat(k);
}

static bool kctrl(void) {
  return IsKeyDown(KEY_LEFT_CONTROL);
}

static bool kalt(void) {
  return IsKeyDown(KEY_LEFT_ALT);
}

static bool kshift(void) {
  return IsKeyDown(KEY_LEFT_SHIFT);
}

static void input_insert_handler(editor_t* E) {
  if (kctrl() && IsKeyPressed(KEY_X)) {
    return mode_set_ctrl_x();
  }
  if (kctrl() && kpr(KEY_A)) {
    return cursor_bol();
  }
  if (kctrl() && kpr(KEY_E)) {
    return cursor_eol(E);
  }
  if (kctrl() && kpr(KEY_K)) {
    return cursor_delete_forward(E);
  }
  if (kctrl() && kpr(KEY_SLASH)) {
    return undo(E);
  }
  if (kctrl() && kpr(KEY_Y)) {
    return cursor_insert_text(E, GetClipboardText());
  }
  if (kctrl() && IsKeyPressed(KEY_SPACE)) {
    return cursor_region_start();
  }
  if (kctrl() && kshift() && IsKeyPressed(KEY_BACKSPACE)) {
    return cursor_delete_row(E);
  }
  if (kalt() && IsKeyPressed(KEY_W)) {
    char* txt = cursor_region_text(E);
    if (txt != NULL) {
      SetClipboardText(txt);
      free(txt);
    }

    return cursor_clear_region();
  }
  if (kctrl() && kpr(KEY_W)) {
    char* txt = cursor_region_kill(E);
    if (txt != NULL) {
      SetClipboardText(txt);
      free(txt);
    }
    return cursor_clear_region();
  }
  if (kalt() && kshift() && kpr(KEY_COMMA)) {
    return cursor_bof();
  }
  if (kalt() && kshift() && kpr(KEY_PERIOD)) {
    return cursor_eof(E);
  }
  if (kalt() && kpr(KEY_F)) {
    return cursor_move_word_forward(E);
  }
  if (kalt() && kpr(KEY_B)) {
    return cursor_move_word_backward(E);
  }
  if (kpr(KEY_ENTER)) {
    return cursor_break_line(E);
  }
  if (kpr(KEY_PAGE_DOWN)) {
    return cursor_page_down(E);
  }
  if (kpr(KEY_PAGE_UP)) {
    return cursor_page_up(E);
  }
  if (kpr(KEY_RIGHT) ||
    (kctrl() && kpr(KEY_F))) {
    cursor_right(E);
  }
  if (kpr(KEY_DOWN) ||
    (kctrl() && kpr(KEY_N))) {
    cursor_down(E);
  }
  if (kpr(KEY_LEFT) ||
    (kctrl() && kpr(KEY_B))) {
    cursor_left(E);
  }
  if (kpr(KEY_UP) ||
    (kctrl() && kpr(KEY_P))) {
    cursor_up(E);
  }
  if (kpr(KEY_BACKSPACE)) {
    cursor_remove_char(E);
  }

  i32 ch = GetCharPressed();
  if (ch >= 32 && ch <= 126)
      cursor_insert_char(E, ch);
}

static void input_command_chain_handler(editor_t* E) {
  if (kctrl()) {
    if (kpr(KEY_C)) {
      if (E->dirty) {
        g_mode = COMMAND_CHAIN;
        mode_set_exit_save(E);
      } else {
        E->running = false;
      }
      return;
    }
    if (kpr(KEY_S)) {
      io_write_buffer(E);
    }
    if (kpr(KEY_G)) {
      return mode_cmd_clean();
    }
  }
}

static void input_command_char_handler(editor_t* E) {
  i32 ch = GetCharPressed();
  if (ch >= 32 && ch <= 126)
    g_active_command.handler(E, ch);
}

void input_keyboard_handler(editor_t* E) {
  if (g_mode & MODE_INSERT)
    input_insert_handler(E);
  else if (g_mode & COMMAND_CHAIN)
    input_command_chain_handler(E);
  else if (g_mode & COMMAND_SINGLE_CHAR)
    input_command_char_handler(E);
}
