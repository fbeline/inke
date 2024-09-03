#include <stdlib.h>
#include <string.h>
#include "../src/cursor.h"
#include "ctest.h"

editor_t editor_factory() {
   editor_t e = {
    .mode = 0,
    .filename = "foo.txt",
    .row_size = 2,
    .row_capacity = 2,
    .dirty = false,
    .new_file = false,
    .running = true,
  };

  e.rows = malloc(sizeof(row_t) * 2);

  char* t1 = "foo bar baz";
  e.rows[0].size = strlen(t1);
  e.rows[0].chars = strdup(t1);

  char* t2 = "qux quux corge";
  e.rows[1].size = strlen(t2);
  e.rows[1].chars = strdup(t2);

  return e;
}

cursor_t factory() {
  editor_t E = editor_factory();
  cursor_t C = {0};
  C.editor = malloc(sizeof(editor_t));
  memcpy(C.editor, &E, sizeof(editor_t));

  return C;
}

static int test_cursor_position(void) {
  cursor_t C = factory();

  ASSERT_VEC2_EQUAL(0, 0, cursor_position(&C));

  C.x = 10; C.coloff = 5;
  C.y = 3; C.rowoff = 20;
  ASSERT_VEC2_EQUAL(16, 23, cursor_position(&C));

  return 0;
}

int main() {
  int result = 0;

  result += test_cursor_position();

  return result;
}
