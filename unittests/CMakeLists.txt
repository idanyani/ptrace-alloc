# for debug
set(CMAKE_VERBOSE_MAKEFILE ON)

# dynamic library
include(GNUInstallDirs)

#project(tracee_l)
#add_library(tracee_l SHARED tracee_lib.c)
#install(TARGETS tracee_l DESTINATION lib)

### Unittests ###
add_executable(unittests ptrace_unittest.cxx PtraceTest.cxx tracee_lib_event_callbacks.cxx)
add_executable(tracee tracee.cxx)
add_dependencies(unittests tracee)

target_link_libraries(tracee tracee_l)
target_link_libraries(unittests tracer gtest gtest_main gmock)
add_test(unittests unittests)

# Temp
add_executable(tracee_test test.cpp PtraceTest.cxx tracee_lib_event_callbacks.cxx)
add_dependencies(tracee_test tracee)
target_link_libraries(tracee_test tracee_l)
target_link_libraries(tracee_test tracer gtest gtest_main gmock)







