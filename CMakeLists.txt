cmake_minimum_required(VERSION 3.0)
project(ptrace_alloc)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pthread")

set(SRC src)

include_directories(${SRC})
add_executable(tracer ${SRC}/main.cxx ${SRC}/ptrace.cxx ${SRC}/syscall.cxx)
add_executable(child_mmap ${SRC}/child_mmap_program.cpp)

add_dependencies(tracer child_mmap)

### GTEST ###
set(GTEST_SRC_DIR googletest)
add_subdirectory(${GTEST_SRC_DIR})
enable_testing()
include_directories(${GTEST_SRC_DIR}/include ${GTEST_SRC_DIR})

### Unittests ###
add_executable(unittests unittests/ptrace_unittest.cxx ${SRC}/ptrace.cxx ${SRC}/syscall.cxx)
add_executable(tracee unittests/tracee.cxx)
add_executable(tracee_mmap unittests/tracee_mmap.cpp)
add_dependencies(unittests tracee tracee_mmap)
target_link_libraries(unittests gtest gtest_main)
add_test(unittests unittests)
