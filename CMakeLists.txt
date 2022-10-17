cmake_minimum_required(VERSION 3.22)

project(lmlogs C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-prototypes -Wold-style-definition -Wstrict-prototypes")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_CXX_FLAGS} -Wmissing-declarations")

add_executable(lmlogs_demo main.c lml_time.h lml_lib.h)