set(PROJECT_SOURCES
  ../src/types.h
  ../src/ds.h  ../src/ds.c
  ../src/globals.h ../src/globals.c
  ../src/utils.h
  ../src/editor.h ../src/editor.c
  ../src/cursor.h ../src/cursor.c
  ../src/undo.h ../src/undo.c
)

file(COPY ${CMAKE_SOURCE_DIR}/tests/samples/
     DESTINATION ${CMAKE_BINARY_DIR}/tests/samples/)

add_executable(editor_test editor_test.c ${PROJECT_SOURCES})
add_executable(cursor_test cursor_test.c ${PROJECT_SOURCES})
add_executable(sample_test sample_test.c ${PROJECT_SOURCES})
add_executable(undo_test undo_test.c ${PROJECT_SOURCES})

add_test(NAME editor_test COMMAND editor_test)
add_test(NAME cursor_test COMMAND cursor_test)
add_test(NAME sample_test COMMAND sample_test)
add_test(NAME undo_test COMMAND undo_test)
