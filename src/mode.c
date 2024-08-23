#include "mode.h"
#include "fs.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

u8 g_mode = MODE_INSERT;
command_t g_active_command = {0};

void mode_exit_save(editor_t* E, char r) {
  char *buf;
  switch (r) {
    case 'y':
      buf = editor_rows_to_string(E->rows, E->row_size);
      FileWrite(E->filename, buf);
      free(buf);
      E->running = false;
    case 'n':
      E->running = false;
    case 'c':
      g_mode = MODE_INSERT;
  }
}

void mode_cmd_not_found(editor_t* E, char r) {
  g_mode = MODE_INSERT;
}

void mode_cmd_nop(editor_t* E, char r) { }

void mode_cmd_clean(void) {
  g_mode = MODE_INSERT;
  g_active_command = (command_t) {
    "",
    &mode_cmd_nop
  };
}

void mode_set_exit_save(editor_t* E) {
  g_mode = MODE_COMMAND;
  g_active_command = (command_t) {
    "Save file? (y/n or [c]ancel)",
    &mode_exit_save
  };
}

void mode_set_ctrl_x(void) {
  g_mode = MODE_COMMAND;
  g_active_command = (command_t) {
    "C-x",
    &mode_cmd_not_found
  };
}

