# for debug
set(CMAKE_VERBOSE_MAKEFILE ON)

# dynamic library
include(GNUInstallDirs)

#project(tracee_l)
#add_library(tracee_l SHARED tracee_lib.c)
#install(TARGETS tracee_l DESTINATION lib)

### Unittests ###
add_executable(unittests ptrace_unittest.cxx PtraceTest.cxx tracee_lib_event_callbacks.cxx test.cpp)
add_executable(tracee tracee.cxx)
add_dependencies(unittests tracee)

target_link_libraries(tracee tracee_l)
target_link_libraries(unittests tracer gtest gtest_main gmock)
add_test(unittests unittests)





