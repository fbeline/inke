#include <stdlib.h>
#include <string.h>
#include "../src/editor.h"
#include "ctest.h"

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

  e.lines = lalloc(0);
  line_append(e.lines, "foo bar baz");

  line_t* l2 = lalloc(0);
  line_append(l2, "qux quux corge");

  e.lines->nl = l2;
  l2->pl = e.lines;

  return e;
}

int test_insert_char_at() {
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

int test_move_line_up() {
  editor_t E = factory();

  // moveup first line
  editor_move_line_up(&E, E.lines);
  ASSERT_STRING_EQUAL("foo bar baz", E.lines->text);
  ASSERT_EQUAL(2, E.row_size);

  // moveup second line
  E.lines = editor_move_line_up(&E, E.lines->nl);
  ASSERT_STRING_EQUAL("foo bar bazqux quux corge", E.lines->text);
  ASSERT_EQUAL(1, E.row_size);

  return 0;
}

int test_delete_rows() {
  editor_t E = factory();

  // delete first line
  editor_delete_lines(&E, E.lines, 1);
  ASSERT_EQUAL(E.row_size, 1);
  ASSERT_STRING_EQUAL("qux quux corge", E.lines->text);

  // delete seconds and last line
  editor_delete_lines(&E, E.lines, 1);
  ASSERT_EQUAL(E.row_size, 1); // min row size is 1
  ASSERT_STRING_EQUAL("", E.lines->text);

  // delete all lines at once
  editor_t E2 = factory();
  editor_delete_lines(&E, E.lines, 2);
  ASSERT_EQUAL(E.row_size, 1); // min row size is 1
  ASSERT_STRING_EQUAL("", E.lines->text);

  return 0;
}

int test_break_line() {
  editor_t E = factory();

  editor_break_line(&E, E.lines, 3);
  ASSERT_EQUAL(E.row_size, 3);
  ASSERT_STRING_EQUAL("foo", E.lines->text);
  ASSERT_EQUAL(3, (i32)E.lines->size);
  ASSERT_STRING_EQUAL(" bar baz", E.lines->nl->text);

  return 0;
}

int test_delete_char() {
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

/* int test_text_between() { */
/*   editor_t E = factory(); */

/*   line_t* lp = editor_text_between(&E, (vec2_t){4, 0}, (vec2_t){7, 0}); */
/*   ASSERT_STRING_EQUAL("bar", lp->text); */

/*   lp = editor_text_between(&E, (vec2_t){4, 1}, (vec2_t){8, 1}); */
/*   ASSERT_STRING_EQUAL("quux", lp->text); */

/*   return 0; */
/* } */

/* int test_cut_between() { */
/*   editor_t E = factory(); */

/*   line_t *lp = editor_cut_between(&E, (vec2_t){4, 0}, (vec2_t){7, 0}); */
/*   ASSERT_STRING_EQUAL("bar", lp->text); */
/*   ASSERT_STRING_EQUAL("foo  baz", E.rows[0].chars); */

/*   lp = editor_cut_between(&E, (vec2_t){4, 1}, (vec2_t){8, 1}); */
/*   ASSERT_STRING_EQUAL("quux", lp->text); */
/*   ASSERT_STRING_EQUAL("qux  corge", E.rows[1].chars); */

/*   return 0; */
/* } */

int test_char_at() {
  editor_t E = factory();

  char ch1 = editor_char_at(E.lines, 4);
  ASSERT_EQUAL('b', ch1);
  char ch2 = editor_char_at(E.lines->nl, E.lines->nl->size-1);
  ASSERT_EQUAL('e', ch2);

  return 0;
}

int test_insert_row_at() {
  editor_t E = factory();

  editor_insert_row_at(&E, 0);
  editor_insert_row_at(&E, 3);
  ASSERT_EQUAL(4, E.row_size);
  ASSERT_STRING_EQUAL("", E.lines->text);
  ASSERT_STRING_EQUAL("foo bar baz", E.lines->nl->text);
  ASSERT_STRING_EQUAL("qux quux corge", E.lines->nl->nl->text);
  ASSERT_STRING_EQUAL("", E.lines->nl->nl->nl->text);

  return 0;
}

int test_insert_row_with_data_at() {
  editor_t E = factory();

  editor_insert_row_with_data_at(&E, 0, "first row");
  ASSERT_STRING_EQUAL("first row", E.lines->text);

  editor_insert_row_with_data_at(&E, 2, "middle row");
  ASSERT_STRING_EQUAL("middle row", E.lines->nl->nl->text);

  return 0;
}

int test_editor_insert_text() {
  editor_t E = factory();

  E.lines = editor_insert_text(E.lines, 0, "first ");
  ASSERT_STRING_EQUAL("first foo bar baz", E.lines->text);

  E.lines = editor_insert_text(E.lines, 6, "middle ");
  ASSERT_STRING_EQUAL("first middle foo bar baz", E.lines->text);

  E.lines = editor_insert_text(E.lines, E.lines->size, " end");
  ASSERT_STRING_EQUAL("first middle foo bar baz end", E.lines->text);

  return 0;
}

int test_rows_to_string() {
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
  /* result += test_text_between(); */
  /* result += test_cut_between(); */
  result += test_char_at();
  result += test_insert_row_at();
  result += test_insert_row_with_data_at();
  result += test_editor_insert_text();
  result += test_rows_to_string();

  return result;
}
