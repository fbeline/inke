#pragma once

#include "utils.h"
#include "cursor.h"

typedef enum {
  ADD, BACKSPACE, CUT, LINEUP, LINEBREAK, LINEDELETE,
  DELETE_FORWARD
} undo_type;

void undo_push(undo_type type, buffer_t *buffer, const char* data);

void undo(buffer_t* B);
