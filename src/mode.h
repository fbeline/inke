#pragma once

#include "cursor.h"

#define MODE_INSERT   0x01
#define MODE_SEARCH   0x02
#define MODE_CMD      0x04
#define MODE_CMD_CHAR 0x08

typedef void (*cmd_func_t)(cursor_t* C, int ch);

void mode_cmd_clean(void);

void mode_set_exit_save(cursor_t* C);

void mode_set_ctrl_x(cursor_t* C);

void mode_set_message(const char* msg, ...);

char* mode_get_message(void);

extern u8 g_mode;
extern cmd_func_t g_cmd_func;
