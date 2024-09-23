#pragma once

#include "utils.h"
#include "cursor.h"

typedef enum {
  ADD, BACKSPACE, CUT, LINEUP, LINEBREAK, LINEDELETE,
  DELETE_FORWARD
} undo_type;

typedef struct undo_s {
  undo_type type;
  cursor_t cursor;
  char* strdata;
  struct undo_s* next;
} undo_t;

void undo_push(undo_type type, cursor_t cursor, const char* data);

undo_t* undo_pop(void);

void undo(cursor_t* C);

void undo_free(undo_t* undo);
