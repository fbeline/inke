#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/editor.h"

#define ASSERT_EQUAL(expected, actual) \
    if ((expected) != (actual)) { \
        printf("Test failed: %s == %s, expected %d but got %d\n", #expected, #actual, (expected), (actual)); \
        return 1; \
    }

#define ASSERT_STRING_EQUAL(expected, actual) \
    if (strcmp((expected), (actual)) != 0) { \
        printf("Test failed: %s == %s\nexpected=\"%s\"\nresult=\"%s\"\n", #expected, #actual, (expected), (actual)); \
        return 1; \
    }

editor_t factory() {
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

int test_insert_char_at() {
  editor_t E = factory();

  // bol
  editor_insert_char_at(&E, 0, 0, 'a');
  ASSERT_STRING_EQUAL("afoo bar baz", E.rows[0].chars);

  // middle
  editor_insert_char_at(&E, 5, 0, 'b');
  ASSERT_STRING_EQUAL("afoo bbar baz", E.rows[0].chars);

  // eol
  editor_insert_char_at(&E, 13, 0, 'y');
  ASSERT_STRING_EQUAL("afoo bbar bazy", E.rows[0].chars);

  return 0;
}

int test_move_line_up() {
  editor_t E = factory();

  // moveup first line
  editor_move_line_up(&E, 0);
  ASSERT_STRING_EQUAL("foo bar baz", E.rows[0].chars);
  ASSERT_EQUAL(2, E.row_size);

  // moveup second line
  editor_move_line_up(&E, 1);
  ASSERT_STRING_EQUAL("foo bar bazqux quux corge", E.rows[0].chars);
  ASSERT_EQUAL(1, E.row_size);

  return 0;
}

int test_delete_rows() {
  editor_t E = factory();

  editor_delete_rows(&E, 0, 0);
  ASSERT_EQUAL(E.row_size, 1);
  editor_delete_rows(&E, 0, 0);
  ASSERT_EQUAL(E.row_size, 0);

  return 0;
}

int test_break_line() {
  editor_t E = factory();

  editor_break_line(&E, 3, 0);
  ASSERT_EQUAL(E.row_size, 3);
  ASSERT_STRING_EQUAL("foo", E.rows[0].chars);
  ASSERT_STRING_EQUAL(" bar baz", E.rows[1].chars);

  return 0;
}

int test_delete_char() {
  editor_t E = factory();

  // first char
  editor_delete_char_at(&E, (vec2_t){0, 0});
  ASSERT_STRING_EQUAL("oo bar baz", E.rows[0].chars);

  // middle line
  editor_delete_char_at(&E, (vec2_t){4, 0});
  ASSERT_STRING_EQUAL("oo ar baz", E.rows[0].chars);

  // last char
  editor_delete_char_at(&E, (vec2_t){strlen(E.rows[0].chars), 0});
  ASSERT_STRING_EQUAL("oo ar ba", E.rows[0].chars);

  return 0;
}

int test_text_between() {
  editor_t E = factory();

  char* str = editor_text_between(&E, (vec2_t){4, 0}, (vec2_t){7, 0});
  ASSERT_STRING_EQUAL("bar", str);

  char* str2 = editor_text_between(&E, (vec2_t){4, 1}, (vec2_t){8, 1});
  ASSERT_STRING_EQUAL("quux", str2);

  return 0;
}

int test_cut_between() {
  editor_t E = factory();

  char* str = editor_cut_between(&E, (vec2_t){4, 0}, (vec2_t){7, 0});
  ASSERT_STRING_EQUAL("bar", str);
  ASSERT_STRING_EQUAL("foo  baz", E.rows[0].chars);

  char* str2 = editor_cut_between(&E, (vec2_t){4, 1}, (vec2_t){8, 1});
  ASSERT_STRING_EQUAL("quux", str2);
  ASSERT_STRING_EQUAL("qux  corge", E.rows[1].chars);

  return 0;
}

int test_char_at() {
  editor_t E = factory();

  char ch1 = editor_char_at(&E, 4, 0);
  ASSERT_EQUAL('b', ch1);
  char ch2 = editor_char_at(&E, strlen(E.rows[1].chars)-1, 1);
  ASSERT_EQUAL('e', ch2);

  return 0;
}

int test_row_len() {
  editor_t E = factory();

  ASSERT_EQUAL(11, (i32)editor_rowlen(&E, 0));

  return 0;
}

int test_insert_row_at() {
  editor_t E = factory();

  // insert row at bof
  editor_insert_row_at(&E, 0);
  ASSERT_EQUAL(3, E.row_size);
  ASSERT_STRING_EQUAL("", E.rows[0].chars);

  // insert row at eof
  editor_insert_row_at(&E, E.row_size);
  ASSERT_EQUAL(4, E.row_size);
  ASSERT_STRING_EQUAL("", E.rows[E.row_size-1].chars);

  return 0;
}

int main() {
  int result = 0;

  result += test_insert_char_at();
  result += test_move_line_up();
  result += test_delete_rows();
  result += test_break_line();
  result += test_delete_char();
  result += test_text_between();
  result += test_cut_between();
  result += test_char_at();
  result += test_row_len();
  result += test_insert_row_at();

  if (result == 0) {
    printf("All tests for editor.c passed!\n");
  } else {
    printf("%d test(s) for editor.c failed.\n", result);
  }

  return result;
}
