include(GNUInstallDirs)

### Unittests ###
add_executable(unittests ptrace_unittest.cxx PtraceTest.cxx)
add_executable(tracee tracee.cxx)
add_dependencies(unittests tracee)

target_link_libraries(tracee tracee_l)
target_link_libraries(unittests tracer gtest gtest_main gmock)
add_test(unittests unittests)

####### Tracee Lib unittests #######
add_executable(tracee_lib_unittests tracee_lib_unittest.cpp tracee_lib_test.cxx PtraceTest.cxx tracee_lib_fifo_test.cxx)

############### DBG ###############
add_executable(dbg tracee_lib_fifo_test.cxx PtraceTest.cxx fifo_mmap_test.cxx)
############### DBG ###############

####### tracees executables ########
add_executable(tracee_basic tracee_basic.c)
target_link_libraries(tracee_basic tracee_l)

add_executable(tracee_fork tracee_fork.c)
target_link_libraries(tracee_fork tracee_l)

add_executable(tracee_fifo tracee_fifo.c)
target_link_libraries(tracee_fifo tracee_l)

add_dependencies(tracee_lib_unittests tracee_basic tracee_fork tracee_fifo)

############### DBG ###############
add_dependencies(dbg tracee_basic tracee_fifo)
target_link_libraries(dbg tracer gtest gtest_main gmock)
############### DBG ###############

target_link_libraries(tracee_lib_unittests tracer gtest gtest_main gmock)
add_test(tracee_lib_unittests tracee_lib_unittests)

# Tarcee lib test
#add_executable(tracee_lib_test tracee_lib_unittest.cxx PtraceTest.cxx tracee_lib_event_callbacks.cxx)
#add_executable(tracee_lib_tracee tracee_lib_test_tracee_programm.cxx)
#target_link_libraries(tracee_lib_tracee tracee_l)

#add_dependencies(tracee_lib_test tracee_lib_tracee)

#target_link_libraries(tracee_lib_test tracer gtest gtest_main gmock)
# add_test(tracee_lib_test tracee_lib_test)







