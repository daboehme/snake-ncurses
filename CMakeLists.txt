cmake_minimum_required(VERSION 3.16)
project(snake C)

find_package(Curses REQUIRED)

add_executable(snake snake.c)
target_link_libraries(snake ${CURSES_LIBRARIES})
