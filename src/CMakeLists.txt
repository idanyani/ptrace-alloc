# for debug
set(CMAKE_VERBOSE_MAKEFILE ON)

add_library(tracer STATIC Ptrace.cxx Syscall.cxx FirstFitAllocator.cpp MemoryIntervalList.cpp
            MosaicRegion.cpp MosaicConfiguration.cpp)
add_executable(ptrace-alloc main.cxx)

set_target_properties(ptrace-alloc PROPERTIES ENABLE_EXPORTS on)


#target_link_libraries(tracer tracee_l_static)
target_link_libraries(tracer tracee_l)
target_link_libraries(ptrace-alloc tracer)
