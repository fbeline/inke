#pragma once

#include "cursor.h"

void mode_cmd_clean(void);

void mode_set_exit_save(cursor_t* C);

void mode_set_ctrl_x(cursor_t* C);

void mode_set_message(const char* msg, ...);

char* mode_get_message(void);
