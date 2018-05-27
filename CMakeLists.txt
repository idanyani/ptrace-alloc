cmake_minimum_required(VERSION 3.0)
project(ptrace_alloc)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

add_executable(tracer main.cxx ptrace.cxx ptrace.h syscallents.h syscall.cxx syscall.h)
add_executable(child_mmap child_mmap_program.cpp)

add_dependencies(tracer child_mmap)
