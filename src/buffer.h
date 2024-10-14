#pragma once

#include "definitions.h"

void buffer_create(const char *filename);

buffer_t *buffer_get(void);

void buffer_next(buffer_t *B);

void buffer_prev(buffer_t *B);

void buffer_save(buffer_t *B);

void buffer_free(buffer_t *B);
