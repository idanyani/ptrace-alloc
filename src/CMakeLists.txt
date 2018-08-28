add_library(tracer STATIC Ptrace.cxx Syscall.cxx FirstFitAllocator.cpp MemoryIntervalList.cpp
            MosaicRegion.cpp MosaicConfiguration.cpp)
add_executable(ptrace-alloc main.cxx)
target_link_libraries(ptrace-alloc tracer)
