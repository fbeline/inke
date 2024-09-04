#include <string.h>

#include "ctest.h"
#include "../src/cursor.h"
#include "../src/editor.h"

static int test_flow(void) {
  editor_t E = editor_init("sample.txt");
  cursor_t C = cursor_init(&E);

  ASSERT_EQUAL(31, E.row_size);

  // DELETE ROW
  cursor_down(&C);
  cursor_delete_row(&C);

  char* l3 = "# Check if a directory was provided as an argument";
  ASSERT_STRING_EQUAL(l3, C.editor->rows[1].chars);
  ASSERT_EQUAL(30, E.row_size);

  // BREAKLINE / ADD NEW ROW
  cursor_move_word_forward(&C);
  cursor_break_line(&C);
  ASSERT_STRING_EQUAL("# Check", C.editor->rows[1].chars);
  ASSERT_STRING_EQUAL(" if a directory was provided as an argument", C.editor->rows[2].chars);
  ASSERT_EQUAL(31, E.row_size);

  return 0;
}

int main() {
  return test_flow();
}
