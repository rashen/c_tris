cmake_minimum_required(VERSION 3.7)

project(TetrisC)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
  -Wall \
  -Werror \
  -Wextra \
  -Wshadow \
  -Wcast-align \
  -Wfloat-equal \
  -Wstrict-prototypes \
  -Wcast-qual \
  -Wswitch-enum \
  -Wconversion \
  ")
set(CMAKE_BUILD_TYPE Debug)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

add_executable(tetris
  src/main.c
  src/render.c
)
target_link_libraries(tetris SDL2::SDL2 SDL2_image::SDL2_image)
