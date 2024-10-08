#pragma once

#include "types.h"

void buffer_create(const char *filename);

buffer_t *buffer_get(void);

buffer_t *buffer_next(void);

buffer_t *buffer_prev(void);

void buffer_free();
