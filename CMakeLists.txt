cmake_minimum_required(VERSION 3.7)

project(TetrisC)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_FLAGS "${CMAKE_FLAGS} -Wall -Werror")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

add_executable(tetris
  src/main.c
  src/render.c
)
target_link_libraries(tetris SDL2::SDL2 SDL2_image::SDL2_image)
