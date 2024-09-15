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

  e.lines = lalloc(0);
  line_append(e.lines, "foo bar baz");

  line_t* l2 = lalloc(0);
  line_append(l2, "qux quux corge");

  e.lines->next = l2;
  l2->prev = e.lines;

  return e;
}

cursor_t factory() {
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
  C.x = C.editor->lines->size - 1;
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
  i32 len = C.clp->size;

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
  i32 len = C.clp->next->size;

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

static int test_move_word_forward(void) {
  cursor_t C = factory();

  cursor_move_word_forward(&C);
  ASSERT_VEC2_EQUAL(3, 0, cursor_position(&C));

  cursor_move_word_forward(&C);
  ASSERT_VEC2_EQUAL(7, 0, cursor_position(&C));

  // move to next line
  cursor_move_word_forward(&C);
  ASSERT_VEC2_EQUAL(3, 1, cursor_position(&C));

  // do not exceed last line last word
  cursor_move_word_forward(&C);
  cursor_move_word_forward(&C);
  cursor_move_word_forward(&C);
  cursor_move_word_forward(&C);
  cursor_move_word_forward(&C);
  ASSERT_VEC2_EQUAL((i32)C.clp->size, 1, cursor_position(&C));

  return 0;
}

static int test_move_word_backward(void) {
  cursor_t C = factory();

  // limit cursor to start of file
  cursor_move_word_backward(&C);
  cursor_move_word_backward(&C);
  ASSERT_VEC2_EQUAL(0, 0, cursor_position(&C));

  cursor_down(&C);
  cursor_eol(&C);

  cursor_move_word_backward(&C);
  ASSERT_VEC2_EQUAL(9, 1, cursor_position(&C));

  // go line up
  cursor_move_word_backward(&C);
  cursor_move_word_backward(&C);
  ASSERT_VEC2_EQUAL(8, 0, cursor_position(&C));

  return 0;
}

static int test_remove_char(void) {
  cursor_t C = factory();

  // do nothing if cursor is at pos.x 0
  cursor_remove_char(&C);
  ASSERT_STRING_EQUAL("foo bar baz", C.clp->text);

  // remove first char of line
  C.x = 1;
  cursor_remove_char(&C);
  ASSERT_STRING_EQUAL("oo bar baz", C.clp->text);

  // remove last char
  cursor_eol(&C);
  cursor_remove_char(&C);
  ASSERT_STRING_EQUAL("oo bar ba", C.clp->text);

  return 0;
}

static int test_insert_char(void) {
  cursor_t C = factory();

  // first char
  cursor_insert_char(&C, 'a');
  ASSERT_STRING_EQUAL("afoo bar baz", C.clp->text);

  // middle
  C.x = 4;
  cursor_insert_char(&C, 'a');
  ASSERT_STRING_EQUAL("afooa bar baz", C.clp->text);

  // last char
  cursor_eol(&C);
  cursor_insert_char(&C, 'a');
  ASSERT_STRING_EQUAL("afooa bar baza", C.clp->text);

  return 0;
}

static int test_insert_text(void) {
  // TODO
  /* cursor_t C = factory(); */
  /* C.x = 4; */

  /* cursor_insert_text(&C, "line1\nline2 "); */
  /* ASSERT_STRING_EQUAL("foo line1", C.clp->text); */
  /* ASSERT_STRING_EQUAL("line2 bar baz", C.clp->next->text); */
  /* ASSERT_EQUAL(3, C.editor->row_size); */

  return 0;
}

static int test_cursor_down(void) {
  cursor_t C = factory();

  cursor_down(&C);
  ASSERT_EQUAL(1, C.y);

  return 0;
}

static int test_cursor_up(void) {
  cursor_t C = factory();

  cursor_up(&C);
  ASSERT_EQUAL(0, C.y);

  cursor_down(&C);
  cursor_up(&C);
  ASSERT_EQUAL(0, C.y);

  return 0;
}


static int test_cursor_right(void) {
  cursor_t C = factory();

  cursor_right(&C);
  ASSERT_VEC2_EQUAL(1, 0, cursor_position(&C));

  // go line down
  C.x = C.clp->size;
  cursor_right(&C);
  ASSERT_VEC2_EQUAL(0, 1, cursor_position(&C));

  return 0;
}

static int test_cursor_left(void) {
  cursor_t C = factory();

  // do nothing when at start of file
  cursor_left(&C);
  ASSERT_VEC2_EQUAL(0, 0, cursor_position(&C));

  C.x = 3;
  cursor_left(&C);
  ASSERT_VEC2_EQUAL(2, 0, cursor_position(&C));

  // goes line up
  cursor_down(&C);
  C.x = 0;
  cursor_left(&C);
  i32 l0len = C.editor->lines->size;
  ASSERT_VEC2_EQUAL(l0len, 0, cursor_position(&C));

  return 0;
}

int test_break_line(void) {
  cursor_t C = factory();

  C.x = 3;
  cursor_break_line(&C);
  ASSERT_STRING_EQUAL("foo", C.clp->prev->text);
  ASSERT_STRING_EQUAL(" bar baz", C.clp->text);

  return 0;
}

int test_delete_forward(void) {
  cursor_t C = factory();

  C.x = 3;
  cursor_delete_forward(&C);
  ASSERT_STRING_EQUAL("foo", C.clp->text);

  return 0;
}

int test_delete_row(void) {
  cursor_t C = factory();

  cursor_delete_row(&C);
  ASSERT_STRING_EQUAL("qux quux corge", C.clp->text);

  // always keep at least 1 line in memory
  cursor_delete_row(&C);
  ASSERT_STRING_EQUAL("", C.clp->text);

  return 0;
}

/* int test_region_text(void) { */
/*   cursor_t C = factory(); */

/*   // single line region */
/*   cursor_region_start(&C); */
/*   C.x = 3; */
/*   char* foo = cursor_region_text(&C); */
/*   ASSERT_STRING_EQUAL("foo", foo); */
/*   C.region.active = false; */

/*   // multline region */
/*   C.x = 4; C.y = 0; */
/*   cursor_region_start(&C); */
/*   C.x = 3; C.y = 1; */
/*   char* str = cursor_region_text(&C); */
/*   ASSERT_STRING_EQUAL("bar baz\nqux", str); */

/*   return 0; */
/* } */

/* int test_region_kill(void) { */
/*   cursor_t C = factory(); */

/*   // single line cut */
/*   cursor_region_start(&C); */
/*   C.x = 3; */
/*   char* foo = cursor_region_kill(&C); */
/*   ASSERT_STRING_EQUAL("foo", foo); */
/*   ASSERT_STRING_EQUAL(" bar baz", C.editor->rows[0].chars); */

/*   // multline cut */
/*   C = factory(); */
/*   C.x = 4; C.y = 0; */
/*   cursor_region_start(&C); */
/*   C.x = 3; C.y = 1; */
/*   char* str = cursor_region_kill(&C); */
/*   ASSERT_STRING_EQUAL("bar baz\nqux", str); */
/*   ASSERT_STRING_EQUAL("foo  quux corge", C.editor->rows[0].chars); */

/*   return 0; */
/* } */

int test_region_eol_bol() {
  cursor_t C = factory();

  cursor_region_start(&C);
  cursor_eol(&C);
  ASSERT_EQUAL((i32)C.clp->size, C.region.size);

  cursor_bol(&C);
  ASSERT_EQUAL(0, C.region.size);

  return 0;
}

int test_region_line_down_up() {
  cursor_t C = factory();

  C.x = 3;
  cursor_region_start(&C);
  cursor_down(&C);
  ASSERT_EQUAL(11, C.region.size);

  cursor_up(&C);
  ASSERT_EQUAL(0, C.region.size);

  return 0;
}

int test_region_left_right() {
  cursor_t C = factory();

  C.x = 2;
  cursor_region_start(&C);
  ASSERT_EQUAL(2, C.region.offset);
  ASSERT_EQUAL(0, C.region.size);

  // testing right
  for(int i = 0; i < 5; i++) {
    cursor_right(&C);
    ASSERT_EQUAL(i + 1, C.region.size);
  }

  // testing left
  i32 size = C.region.size;
  for(int i = 0; i < 4; i++) {
    cursor_left(&C);
    ASSERT_EQUAL(--size, C.region.size);
  }

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
  result += test_move_word_forward();
  result += test_move_word_backward();
  result += test_remove_char();
  result += test_insert_char();
  result += test_insert_text();
  result += test_cursor_up();
  result += test_cursor_right();
  result += test_cursor_down();
  result += test_cursor_left();
  result += test_break_line();
  result += test_delete_forward();
  result += test_delete_row();
  result += test_region_eol_bol();
  result += test_region_line_down_up();
  result += test_region_left_right();
  /* result += test_region_text(); */
  /* result += test_region_kill(); */

  return result;
}
