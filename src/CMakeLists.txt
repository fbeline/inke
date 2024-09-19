set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(inke
  main.c
  types.h
  globals.h globals.c
  io.h io.c
  vt100.h vt100.c
  terminal.h terminal.c
  utils.h utils.c
  input.h input.c
  undo.h undo.c
  mode.h mode.c
  cursor.h cursor.c
  editor.h editor.c)

if(WIN32)
  target_link_libraries(inke)
endif()

if(UNIX)
  target_link_libraries(inke -fsanitize=undefined,address -Wall -Wextra -pedantic)
endif()
