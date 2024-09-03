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
  C.max_col = 9999;
  C.max_row = 9999;
  C.editor = malloc(sizeof(editor_t));
  memcpy(C.editor, &E, sizeof(editor_t));

  return C;
}

static int test_cursor_position(void) {
  cursor_t C = factory();

  ASSERT_VEC2_EQUAL(0, 0, cursor_position(&C));

  C.x = 10; C.coloff = 5;
  C.y = 3; C.rowoff = 20;
  ASSERT_VEC2_EQUAL(15, 23, cursor_position(&C));

  return 0;
}

static int test_cursor_char(void) {
  cursor_t C = factory();

  // first char
  ASSERT_EQUAL('f', cursor_char(&C));

  // middle line
  C.x = 4;
  ASSERT_EQUAL('b', cursor_char(&C));

  // last char
  C.x = strlen(C.editor->rows[0].chars) - 1;
  ASSERT_EQUAL('z', cursor_char(&C));

  return 0;
}

static int test_bol(void) {
  cursor_t C = factory();
  C.x = 10;
  C.coloff = 5;

  cursor_bol(&C);

  ASSERT_EQUAL(0, C.x);
  ASSERT_EQUAL(0, C.coloff);

  return 0;
}

static int test_eol(void) {
  cursor_t C = factory();
  i32 len = strlen(C.editor->rows[0].chars);

  // maxcol 9999
  cursor_eol(&C);
  ASSERT_EQUAL(len, C.x);

  // maxcol 4
  C.x = 0;
  C.max_col = 4;
  cursor_eol(&C);
  ASSERT_EQUAL(C.max_col, C.x);
  ASSERT_EQUAL(len - C.max_col, C.coloff);


  return 0;
}

static int test_eof(void) {
  cursor_t C = factory();
  i32 len = strlen(C.editor->rows[1].chars);

  cursor_eof(&C);
  ASSERT_EQUAL(1, C.y);
  ASSERT_EQUAL(0, C.rowoff);
  ASSERT_EQUAL(len, C.x);

  // row size < max row
  cursor_t C2 = factory();
  C2.max_row = 1;
  cursor_eof(&C2);
  ASSERT_EQUAL(0, C2.y);
  ASSERT_EQUAL(1, C2.rowoff);
  ASSERT_EQUAL(len, C.x);

  return 0;
}

static int test_bof(void) {
  cursor_t C = factory();
  C.x = 20; C.coloff = 30;
  C.y = 10; C.rowoff = 15;

  cursor_bof(&C);
  ASSERT_VEC2_EQUAL(0, 0, cursor_position(&C));

  return 0;
}

int main() {
  int result = 0;

  result += test_cursor_position();
  result += test_cursor_char();
  result += test_bol();
  result += test_eol();
  result += test_eof();
  result += test_bof();

  return result;
}
