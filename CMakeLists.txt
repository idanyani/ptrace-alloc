cmake_minimum_required(VERSION 3.0)
project(ptrace_alloc)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

add_executable(tracer main.cxx)
add_executable(child_mmap child_mmap_program.cpp)
