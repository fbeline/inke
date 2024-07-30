cmake_minimum_required(VERSION 3.24)
project(olive)
set(CMAKE_C_STANDARD 99)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(raylib CONFIG REQUIRED)

add_executable(olive
  src/main.c
  src/types.h
  src/utils.h src/utils.c
  src/editor.h src/editor.c
  src/render.h src/render.c
  src/input.h src/input.c
  src/fs.h src/fs.c)

if(WIN32)
  target_link_libraries(olive PRIVATE raylib winmm)
endif()

if(UNIX)
  target_link_libraries(olive PRIVATE raylib -fsanitize=undefined -fsanitize=address)
endif()
