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

int main() {
    int result = 0;

    result += test_insert_char_at();
    result += test_move_line_up();

    if (result == 0) {
        printf("All tests for editor.c passed!\n");
    } else {
        printf("%d test(s) for editor.c failed.\n", result);
    }

    return result;
}
