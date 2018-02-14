cmake_minimum_required(VERSION 3.8)
project(gameOfLife)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES program.c)
add_executable(program2 ${SOURCE_FILES})