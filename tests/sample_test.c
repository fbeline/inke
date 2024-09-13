#include <string.h>

#include "ctest.h"
#include "../src/cursor.h"
#include "../src/editor.h"

static int test_flow(void) {
  editor_t E = editor_init("samples/sample.txt");
  cursor_t C = cursor_init(&E);

  ASSERT_EQUAL(31, E.row_size);

  // DELETE ROW
  cursor_down(&C);
  cursor_delete_row(&C);

  char* l3 = "# Check if a directory was provided as an argument";
  ASSERT_STRING_EQUAL(l3, C.clp->text);
  ASSERT_EQUAL(30, E.row_size);

  return 0;
}

static int test_empty_file(void) {
  editor_t E = editor_init("samples/empty_sample.txt");
  cursor_t C = cursor_init(&E);

  ASSERT_EQUAL(1, E.row_size);

  return 0;
}

int main() {
  return test_flow() + test_empty_file();
}
