#include "undo.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "globals.h"
#include "editor.h"

static undo_t* undo_head = NULL;

void undo_push(undo_type type, cursor_t cursor, const char* data) {
  if (g_undo_state == UNDO_OFF) return;

  undo_t* undo = (undo_t*)malloc(sizeof(undo_t));
  undo->type = type;
  undo->cursor = cursor;
  undo->next = undo_head;
  undo->strdata = NULL;
  if (data != NULL) {
    usize size = strlen(data);
    undo->strdata = malloc(size + 1);
    memcpy(undo->strdata, data, size);
    undo->strdata[size] = '\0';
  }

  undo_head = undo;
}

undo_t* undo_pop(void) {
  undo_t* head = undo_head;
  if (head == NULL) return NULL;

  undo_head = head->next;
  return head;
}

static undo_t *undo_peek(void) {
  return undo_head;
}

void undo_free(undo_t* undo) {
  if (undo == NULL) return;
  if (undo->strdata != NULL) free(undo->strdata);

  free(undo);
}

static void undo_line_delete(cursor_t *C, undo_t *undo) {
  usize y = C->y + C->rowoff;
  C->clp = editor_insert_row_with_data_at(C->editor, y, undo->strdata);

  if (y == 0) C->editor->lines = C->clp;
}

static void undo_undo(cursor_t *C, undo_t *u) {
  undo_t *next = undo_peek();
  if (next == NULL || next->type != u->type)
    return;

  undo(C);
}

void undo(cursor_t* C) {
  undo_t* u = undo_pop();
  if (u == NULL) return;

  cursor_set(C, &u->cursor);
  g_mode = MODE_INSERT;

  g_undo_state = UNDO_OFF;
  switch (u->type) {
    case ADD:
      cursor_remove_char(C);
      break;
    case BACKSPACE:
      cursor_insert_char(C, u->strdata[0]);
      break;
    case LINEUP:
      cursor_break_line(C);
      break;
    case LINEBREAK:
      cursor_move_line_up(C);
      break;
    case LINEDELETE:
      undo_line_delete(C, u);
      break;
    case DELETE_FORWARD:
      cursor_insert_text(C, u->strdata);
      break;
    case CUT:
      cursor_insert_text(C, u->strdata);
      break;
    default:
      printf("UNDO TYPE NOT IMPLEMENTED %d\n", u->type);
  }

  undo_undo(C, u);
  g_undo_state = UNDO_ON;
  undo_free(u);
}

