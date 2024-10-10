#pragma once

#include "types.h"

void prompt_init(const char *msg);
void prompt_cat(const char *str);
void prompt_insert(char ch);
void prompt_backspace(void);
void prompt_clean(void);
void prompt_left(void);
void prompt_right(void);
void prompt_eol(void);
void prompt_bol(void);
void prompt_del_before(void);
u32 prompt_x(void);
const char *prompt_text(void);
const char *prompt_full_text(void);
