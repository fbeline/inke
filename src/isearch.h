#pragma once

#include "types.h"

void isearch(cursor_t *C, int ch);
void isearch_forward(cursor_t *C, const char *text);
void isearch_reverse(cursor_t *C, const char *text);
