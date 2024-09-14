#include "undo.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define UNDO_OFF 0
#define UNDO_ON  1

static undo_t* undo_head = NULL;
static int undo_state = UNDO_ON;

void undo_push(undo_type type, cursor_t cursor, const char* data) {
  if (undo_state == UNDO_OFF) return;

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

void undo_free(undo_t* undo) {
  if (undo == NULL) return;
  if (undo->strdata != NULL) free(undo->strdata);

  free(undo);
}

void undo(cursor_t* C) {
  editor_t* E = C->editor;
  undo_t* undo = undo_pop();
  if (undo == NULL) return;

  cursor_set(C, &undo->cursor);
  C->region.active = false;

  undo_state = UNDO_OFF;
  switch (undo->type) {
    case ADD:
      cursor_remove_char(C);
      break;
    case BACKSPACE:
      cursor_insert_char(C, undo->strdata[0]);
      break;
    case LINEUP:
      cursor_break_line(C);
      break;
    case LINEBREAK:
      cursor_move_line_up(C);
      break;
    case LINEDELETE:
      /* editor_insert_row_with_data_at(E, undo->pos.y, undo->strdata); */
      break;
    case DELETE_FORWARD:
      cursor_insert_text(C, undo->strdata);
      break;
    case CUT:
      cursor_insert_text(C, undo->strdata);
      break;
    case PASTE:
      // TODO
      break;
    default:
      printf("UNDO TYPE NOT IMPLEMENTED %d\n", undo->type);
  }

  undo_state = UNDO_ON;
  undo_free(undo);
}

