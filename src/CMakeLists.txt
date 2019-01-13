add_library(tracer STATIC Ptrace.cxx Syscall.cxx FirstFitAllocator.cpp MemoryIntervalList.cpp
            MosaicRegion.cpp MosaicConfiguration.cpp)
add_executable(ptrace-alloc main.cxx ../unittests/tracee_lib_event_callbacks.cxx ../unittests/tracee_lib_event_callbacks.h ../include/setUserSignalHandlers.h)
target_link_libraries(ptrace-alloc tracer)
