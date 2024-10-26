#pragma once

#include "definitions.h"

void isearch_search(buffer_t *B, const char *query, u8 dir);
void isearch_start(buffer_t *B);
void isearch(i32 opt);
void isearch_abort(buffer_t *B);
