#pragma once

#include "types.h"

void cmdline_init(const char *msg);
void cmdline_cat(const char *str);
void cmdline_insert(char ch);
void cmdline_backspace(void);
void cmdline_clean(void);
cmdline_t *cmdline(void);
