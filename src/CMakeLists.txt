cmake_minimum_required(VERSION 3.24)
project(olive)
set(CMAKE_C_STANDARD 99)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(raylib CONFIG REQUIRED)

add_executable(olive
  main.c
  types.h
  io.h io.c
  utils.h utils.c
  undo.h undo.c
  mode.h mode.c
  cursor.h cursor.c
  editor.h editor.c
  render.h render.c
  input.h input.c)

if(WIN32)
  target_link_libraries(olive PRIVATE raylib winmm)
endif()

if(UNIX)
  target_link_libraries(olive PRIVATE raylib -fsanitize=undefined -fsanitize=address)
endif()
