cmake_minimum_required(VERSION 3.24)
project(olive)
set(CMAKE_C_STANDARD 99)
# set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(raylib CONFIG REQUIRED)

add_executable(olive src/main.c)

target_link_libraries(olive PRIVATE raylib winmm)
