set(PROJECT_SOURCES
  ../src/utils.h ../src/utils.c
  ../src/editor.h ../src/editor.c
  ../src/cursor.h ../src/cursor.c
  ../src/undo.h ../src/undo.c
)

add_executable(editor_test editor_test.c ${PROJECT_SOURCES})
add_executable(cursor_test cursor_test.c ${PROJECT_SOURCES})

add_test(NAME editor_test COMMAND editor_test)
add_test(NAME cursor_test COMMAND cursor_test)
