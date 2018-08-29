
### Unittests ###
add_executable(unittests ptrace_unittest.cxx PtraceTest.cxx)
add_executable(tracee tracee.cxx)
add_dependencies(unittests tracee)

target_link_libraries(unittests tracer gtest gtest_main gmock)
add_test(unittests unittests)
