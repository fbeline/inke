#include "undo.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "cursor.h"
#include "definitions.h"
#include "ds.h"
#include "editor.h"
#include "globals.h"

void undo_push(undo_type type, buffer_t *buffer, const char* data) {
  if (!(g_flags & UNDO)) return;

  undo_t *head = buffer->up;
  if (type == BACKSPACE &&
      head != NULL && head->type == type &&
      raw_y(&buffer->cursor) == raw_y(&head->cursor) &&
      raw_x(&buffer->cursor) + 1 == raw_x(&head->cursor)
  ) {
    dsichar(head->strdata, 0, data[0]);
    buffer->up->cursor = buffer->cursor;
    return;
  }

  if (type == ADD &&
      head != NULL && head->type == type &&
      raw_y(&buffer->cursor) == raw_y(&head->cursor) &&
      raw_x(&buffer->cursor) == raw_x(&head->cursor) + 1
  ) {
    buffer->up->n++;
    buffer->up->cursor = buffer->cursor;
    return;
  }

  undo_t* undo = (undo_t*)malloc(sizeof(undo_t));
  undo->type = type;
  undo->cursor = buffer->cursor;
  undo->next = head;
  undo->strdata = dsnew(data);
  undo->n = 1;

  buffer->up = undo;
}

static undo_t* undo_pop(buffer_t *B) {
  undo_t* head = B->up;
  if (head == NULL) return NULL;

  B->up = head->next;
  return head;
}

static undo_t *undo_peek(buffer_t *B) {
  return B->up;
}

void undo_free(undo_t *undo) {
  if (undo == NULL) return;
  dsfree(undo->strdata);
  free(undo);
}

static void undo_line_delete(buffer_t *B, undo_t *undo) {
  cursor_t *C = &B->cursor;
  usize y = C->y + C->rowoff;
  B->lp = editor_insert_row_with_data_at(&B->editor, y, undo->strdata->buf);

  if (y == 0) B->editor.lines = B->lp;
}

void undo(buffer_t *B) {
  undo_t* u = undo_pop(B);
  if (u == NULL) return;

  cursor_set(B, &u->cursor);

  g_flags &= ~UNDO;
  switch (u->type) {
    case ADD:
      for (u32 i = 0; i < u->n; i++) {
        cursor_remove_char(B);
      }
      break;
    case BACKSPACE:
      cursor_insert_text(B, u->strdata->buf);
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
      cursor_insert_text(B, u->strdata->buf);
      break;
    case CUT:
      cursor_insert_text(B, u->strdata->buf);
      break;
    default:
      printf("UNDO TYPE NOT IMPLEMENTED %d\n", u->type);
  }

  g_flags |= UNDO;
  undo_free(u);
}

