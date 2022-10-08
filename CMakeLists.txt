cmake_minimum_required(VERSION 3.22)

project(lmlogs C)

set(CMAKE_C_STANDARD 99)

add_executable(lmlogs_demo main.c lml_time.h lml_lib.h)