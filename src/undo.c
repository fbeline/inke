#include "undo.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static undo_t* undo_head = NULL;

void undo_push(undo_type type, vec2_t pos, cursor_t cursor, const char* data) {
  undo_t* undo = (undo_t*)malloc(sizeof(undo_t));
  undo->type = type;
  undo->pos = pos;
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

void undo(editor_t* E) {
  undo_t* undo = undo_pop();

  if (undo == NULL) return;

  switch (undo->type) {
    case ADD:
      editor_delete_char_at(E, undo->pos);
      break;
    case BACKSPACE:
      editor_insert_char_at(E, undo->pos.x, undo->pos.y, undo->strdata[0]);
      break;
    case LINEUP:
      editor_break_line(E, undo->pos.x, undo->pos.y);
      break;
    case LINEBREAK:
      editor_move_line_up(E, undo->pos.y);
      break;
    case LINEDELETE:
      editor_insert_row_with_data_at(E, undo->pos.y, undo->strdata);
      break;
    case DELETE_FORWARD:
      editor_insert_text(E, undo->pos, undo->strdata, strlen(undo->strdata));
      break;

    default:
      printf("UNDO TYPE NOT IMPLEMENTED %d\n", undo->type);
      undo_free(undo);
      return;
  }

  cursor_set(&undo->cursor);
  undo_free(undo);
}

