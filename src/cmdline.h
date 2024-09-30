#pragma once

#include "types.h"

void cmdline_init(const char *msg);
void cmdline_cat(const char *str);
void cmdline_insert(char ch);
void cmdline_backspace(void);
void cmdline_clean(void);
void cmdline_left(void);
void cmdline_right(void);
void cmdline_eol(void);
void cmdline_bol(void);
void cmdline_del_before(void);
cmdline_t *cmdline(void);
