#pragma once

#define MODE_INSERT          0x01
#define MODE_SEARCH          0x02
#define COMMAND_SINGLE_CHAR  0x04
#define COMMAND_STRING_INPUT 0x08
#define COMMAND_CHAIN        0x10
#define COMMAND_EMPTY        0x00

#include "editor.h"

typedef void (*command_handler_t)(editor_t* E, char r);

typedef struct command_s {
  char* text;
  command_handler_t handler;
} command_t;

void mode_cmd_clean(void);

void mode_set_exit_save(editor_t* E);

void mode_set_ctrl_x(void);

extern u8 g_mode;
extern command_t g_active_command;
