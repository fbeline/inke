set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(olive
  main.c
  types.h
  io.h io.c
  utils.h utils.c
  undo.h undo.c
  mode.h mode.c
  cursor.h cursor.c
  editor.h editor.c)

if(WIN32)
  target_link_libraries(olive)
endif()

if(UNIX)
  target_link_libraries(olive -fsanitize=undefined,address)
endif()
