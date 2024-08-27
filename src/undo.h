#pragma once

#include "utils.h"
#include "editor.h"
#include "cursor.h"

typedef enum {
  ADD, DEL, ENTER, BACK, CUT, PASTE
} undo_type;

typedef struct undo_s {
  undo_type type;
  vec2_t pos;
  cursor_t cursor;
  char* strdata;
  struct undo_s* next;
} undo_t;

void undo_push(undo_type type, vec2_t pos, cursor_t cursor, const char* data);

undo_t* undo_pop(void);

void undo(editor_t* E);

void undo_free(undo_t* undo);


