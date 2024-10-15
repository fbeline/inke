#include "undo.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "globals.h"
#include "editor.h"

typedef struct undo_s {
  undo_type type;
  cursor_t cursor;
  char* strdata;

  struct undo_s* next;
} undo_t;

static undo_t* undo_head = NULL;

void undo_push(undo_type type, buffer_t *buffer, const char* data) {
  if (!(g_flags & UNDO)) return;

  undo_t* undo = (undo_t*)malloc(sizeof(undo_t));
  undo->type = type;
  undo->cursor = buffer->cursor;
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

static undo_t* undo_pop(void) {
  undo_t* head = undo_head;
  if (head == NULL) return NULL;

  undo_head = head->next;
  return head;
}

static undo_t *undo_peek(void) {
  return undo_head;
}

static void undo_free(undo_t* undo) {
  if (undo == NULL) return;
  if (undo->strdata != NULL) free(undo->strdata);

  free(undo);
}

static void undo_line_delete(buffer_t *B, undo_t *undo) {
  cursor_t *C = &B->cursor;
  usize y = C->y + C->rowoff;
  B->lp = editor_insert_row_with_data_at(&B->editor, y, undo->strdata);

  if (y == 0) B->editor.lines = B->lp;
}

static void undo_undo(buffer_t *B, undo_t *u) {
  undo_t *next = undo_peek();
  if (next == NULL || next->type != u->type)
    return;

  undo(B);
}

void undo(buffer_t *B) {
  undo_t* u = undo_pop();
  if (u == NULL) return;

  cursor_set(B, &u->cursor);

  g_flags &= ~UNDO;
  switch (u->type) {
    case ADD:
      cursor_remove_char(B);
      break;
    case BACKSPACE:
      cursor_insert_char(B, u->strdata[0]);
      break;
    case LINEUP:
      cursor_break_line(B);
      break;
    case LINEBREAK:
      cursor_move_line_up(B);
      break;
    case LINEDELETE:
      undo_line_delete(B, u);
      break;
    case DELETE_FORWARD:
      cursor_insert_text(B, u->strdata);
      break;
    case CUT:
      cursor_insert_text(B, u->strdata);
      break;
    default:
      printf("UNDO TYPE NOT IMPLEMENTED %d\n", u->type);
  }

  undo_undo(B, u);
  g_flags |= UNDO;
  undo_free(u);
}

