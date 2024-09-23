#include <stdlib.h>
#include <string.h>

#include "ctest.h"
#include "../src/editor.h"
#include "../src/globals.h"

editor_t factory(void) {
   editor_t e = {
    .mode = 0,
    .filename = "foo.txt",
    .row_size = 2,
    .dirty = false,
    .new_file = false,
  };

  e.lines = lalloc(0);
  line_append(e.lines, "foo bar baz");

  line_t* l2 = lalloc(0);
  line_append(l2, "qux quux corge");

  e.lines->next = l2;
  l2->prev = e.lines;

  return e;
}

static int test_insert_char_at(void) {
  editor_t E = factory();

  // bol
  editor_insert_char_at(&E, E.lines, 0, 'a');
  ASSERT_STRING_EQUAL("afoo bar baz", E.lines->text);

  // middle
  editor_insert_char_at(&E, E.lines, 5, 'b');
  ASSERT_STRING_EQUAL("afoo bbar baz", E.lines->text);

  // eol
  editor_insert_char_at(&E, E.lines, 13, 'y');
  ASSERT_STRING_EQUAL("afoo bbar bazy", E.lines->text);

  return 0;
}

static int test_move_line_up(void) {
  editor_t E = factory();

  // moveup first line
  editor_move_line_up(&E, E.lines);
  ASSERT_STRING_EQUAL("foo bar baz", E.lines->text);
  ASSERT_EQUAL(2ul, E.row_size);

  // moveup second line
  E.lines = editor_move_line_up(&E, E.lines->next);
  ASSERT_STRING_EQUAL("foo bar bazqux quux corge", E.lines->text);
  ASSERT_EQUAL(1ul, E.row_size);

  return 0;
}

static int test_delete_rows(void) {
  editor_t E = factory();

  // delete first line
  editor_delete_lines(&E, E.lines, 1);
  ASSERT_EQUAL(E.row_size, 1ul);
  ASSERT_STRING_EQUAL("qux quux corge", E.lines->text);

  // delete seconds and last line
  editor_delete_lines(&E, E.lines, 1);
  ASSERT_EQUAL(E.row_size, 1ul); // min row size is 1
  ASSERT_STRING_EQUAL("", E.lines->text);

  // delete all lines at once
  editor_t E2 = factory();
  editor_delete_lines(&E2, E.lines, 2);
  ASSERT_EQUAL(E2.row_size, 1ul); // min row size is 1
  ASSERT_STRING_EQUAL("", E2.lines->text);

  return 0;
}

static int test_break_line(void) {
  editor_t E = factory();

  editor_break_line(&E, E.lines, 3);
  ASSERT_EQUAL(E.row_size, 3ul);
  ASSERT_STRING_EQUAL("foo", E.lines->text);
  ASSERT_EQUAL(3ul, E.lines->size);
  ASSERT_STRING_EQUAL(" bar baz", E.lines->next->text);

  return 0;
}

static int test_delete_char(void) {
  editor_t E = factory();

  // first char
  editor_delete_char_at(E.lines, 0);
  ASSERT_STRING_EQUAL("oo bar baz", E.lines->text);

  // middle line
  editor_delete_char_at(E.lines, 3);
  ASSERT_STRING_EQUAL("oo ar baz", E.lines->text);

  // last char
  editor_delete_char_at(E.lines, strlen(E.lines->text)-1);
  ASSERT_STRING_EQUAL("oo ar ba", E.lines->text);

  return 0;
}

static int test_text_between(void) {
  editor_t E = factory();

  mark_t mark = {
    .start_lp = E.lines,
    .start_offset = 4,
    .end_lp = E.lines,
    .end_offset = 7
  };

  ds_t *ds = dsempty();
  editor_text_between(&E, mark, ds);
  ASSERT_STRING_EQUAL("bar", ds->buf);

  mark.end_lp = E.lines->next;
  mark.end_offset = 3;

  ds_t *ds2 = dsempty();
  editor_text_between(&E, mark, ds2);
  ASSERT_STRING_EQUAL("bar baz\nqux", ds2->buf);

  dsfree(ds);
  dsfree(ds2);

  return 0;
}

static int test_kill_between(void) {
  /* editor_t E = factory(); */

  /* char *text = editor_kill_between(&E, E.lines, 4, 3); */
  /* ASSERT_STRING_EQUAL("bar", text); */
  /* ASSERT_STRING_EQUAL("foo  baz", E.lines->text); */

  /* editor_t E2 = factory(); */
  /* char *text2 = editor_kill_between(&E2, E2.lines, 4, 11); */
  /* ASSERT_STRING_EQUAL("foo quux corge", E2.lines->text); */

  return 0;
}

static int test_char_at(void) {
  editor_t E = factory();

  char ch1 = editor_char_at(E.lines, 4);
  ASSERT_EQUAL('b', ch1);
  char ch2 = editor_char_at(E.lines->next, E.lines->next->size-1);
  ASSERT_EQUAL('e', ch2);

  return 0;
}

static int test_insert_row_at(void) {
  editor_t E = factory();

  editor_insert_row_at(&E, 0);
  editor_insert_row_at(&E, 3);
  ASSERT_EQUAL(4lu, E.row_size);
  ASSERT_STRING_EQUAL("", E.lines->text);
  ASSERT_STRING_EQUAL("foo bar baz", E.lines->next->text);
  ASSERT_STRING_EQUAL("qux quux corge", E.lines->next->next->text);
  ASSERT_STRING_EQUAL("", E.lines->next->next->next->text);

  return 0;
}

static int test_insert_row_with_data_at(void) {
  editor_t E = factory();

  editor_insert_row_with_data_at(&E, 0, "first row");
  ASSERT_STRING_EQUAL("first row", E.lines->text);

  editor_insert_row_with_data_at(&E, 2, "middle row");
  ASSERT_STRING_EQUAL("middle row", E.lines->next->next->text);

  return 0;
}

static int test_editor_insert_text(void) {
  editor_t E = factory();

  editor_insert_text(&E, E.lines, 0, "first ");
  ASSERT_STRING_EQUAL("first foo bar baz", E.lines->text);

  editor_insert_text(&E, E.lines, 6, "middle ");
  ASSERT_STRING_EQUAL("first middle foo bar baz", E.lines->text);

  editor_insert_text(&E, E.lines, E.lines->size, " end");
  ASSERT_STRING_EQUAL("first middle foo bar baz end", E.lines->text);

  editor_t E2 = factory();
  editor_insert_text(&E2, E2.lines, 4, "testing\nmultiline ");
  ASSERT_STRING_EQUAL(E2.lines->text, "foo testing");
  ASSERT_STRING_EQUAL(E2.lines->next->text, "multiline bar baz");
  ASSERT_EQUAL(3lu, E2.row_size);

  return 0;
}

static int test_rows_to_string(void) {
  editor_t E = factory();

  line_t* lp = editor_rows_to_string(E.lines, E.row_size);
  ASSERT_STRING_EQUAL("foo bar baz\nqux quux corge\n", lp->text);

  line_free(lp);

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
  result += test_kill_between();
  result += test_char_at();
  result += test_insert_row_at();
  result += test_insert_row_with_data_at();
  result += test_editor_insert_text();
  result += test_rows_to_string();

  return result;
}
