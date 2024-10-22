#pragma once

#include "utils.h"
#include "cursor.h"

void undo_push(undo_type type, buffer_t *buffer, const char* data);

void undo(buffer_t* B);

void undo_free(undo_t *u);
