cmake_minimum_required(VERSION 3.7)

project(TetrisC)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_FLAGS "${CMAKE_FLAGS} -Wall -Werror")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(tetris main.c)
target_link_libraries(tetris ${SDL2_LIBRARIES})
