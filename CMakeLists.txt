cmake_minimum_required(VERSION 3.0)
project(ptrace_alloc)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pthread")

include_directories(src)
add_subdirectory(src)
add_subdirectory(unittests)
add_subdirectory(examples)

### GTEST ###
set(GTEST_SRC_DIR googletest)
add_subdirectory(${GTEST_SRC_DIR})
enable_testing()
include_directories(${GTEST_SRC_DIR}/include ${GTEST_SRC_DIR})
