cmake_minimum_required(VERSION 3.0)
project(ptrace_alloc)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

set(SRC src)

include_directories(${SRC})
add_executable(tracer ${SRC}/main.cxx ${SRC}/ptrace.cxx ${SRC}/syscall.cxx)
add_executable(child_mmap ${SRC}/child_mmap_program.cpp)

add_dependencies(tracer child_mmap)

# TODO: can we use: add_test()?
add_executable(ptrace_test test/ptrace_test.cxx ${SRC}/ptrace.cxx ${SRC}/syscall.cxx)
add_executable(tracee test/tracee.cxx)

add_dependencies(ptrace_test tracee)
