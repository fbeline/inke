set(PROJECT_SOURCES
  ../src/utils.h ../src/utils.c
  ../src/editor.h ../src/editor.c
)

add_executable(editor_test editor_test.c ${PROJECT_SOURCES})
add_test(NAME editor_test COMMAND editor_test)
