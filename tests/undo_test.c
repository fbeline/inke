#include <string.h>
#include <stdlib.h>

#include "ctest.h"
#include "../src/cursor.h"
#include "../src/editor.h"
#include "../src/undo.h"

static editor_t editor_factory(void) {
   editor_t e = {
    .mode = 0,
    .filename = "foo.txt",
    .row_size = 2,
    .row_capacity = 2,
    .dirty = false,
    .new_file = false,
    .running = true,
  };

  e.lines = lalloc(0);
  line_append(e.lines, "foo bar baz");

  line_t* l2 = lalloc(0);
  line_append(l2, "qux quux corge");

  e.lines->next = l2;
  l2->prev = e.lines;

  return e;
}

static cursor_t factory(void) {
  editor_t E = editor_factory();
  cursor_t C = {0};
  C.region.active = false;
  C.max_col = 9999;
  C.max_row = 9999;
  C.region.cursor = malloc(sizeof(cursor_t));
  C.editor = malloc(sizeof(editor_t));
  memcpy(C.editor, &E, sizeof(editor_t));

  C.clp = C.editor->lines;

  return C;
}

static int test_undo_add(void) {
  cursor_t C = factory();

  cursor_eol(&C);
  cursor_insert_char(&C, 'a');
  cursor_insert_char(&C, 'b');
  cursor_insert_char(&C, 'c');

  ASSERT_STRING_EQUAL("foo bar bazabc", C.clp->text);

  undo(&C);
  ASSERT_STRING_EQUAL("foo bar bazab", C.clp->text);
  undo(&C);
  ASSERT_STRING_EQUAL("foo bar baza", C.clp->text);
  undo(&C);
  ASSERT_STRING_EQUAL("foo bar baz", C.clp->text);

  return 0;
}

static int test_undo_backspace(void) {
  cursor_t C = factory();

  cursor_eol(&C);
  cursor_remove_char(&C);
  cursor_remove_char(&C);
  ASSERT_STRING_EQUAL("foo bar b", C.clp->text);

  undo(&C);
  ASSERT_STRING_EQUAL("foo bar ba", C.clp->text);
  undo(&C);
  ASSERT_STRING_EQUAL("foo bar baz", C.clp->text);

  // lineup
  cursor_down(&C);
  cursor_bol(&C);
  cursor_remove_char(&C);
  ASSERT_STRING_EQUAL("foo bar bazqux quux corge", C.clp->text);

  return 0;
}

int main() {
  int result = 0;
  result += test_undo_add();
  result += test_undo_backspace();

  return result;
}