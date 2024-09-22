set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(inke
  main.c
  types.h
  ds.h ds.c
  globals.h globals.c
  io.h io.c
  vt.h vt.c
  terminal.h terminal.c
  utils.h utils.c
  input.h input.c
  undo.h undo.c
  mode.h mode.c
  cursor.h cursor.c
  editor.h editor.c)
