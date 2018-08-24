#include <iostream>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>

#include "memory_interval_list.h"

int main() {
    MemoryIntervalList l;
    l.Initialize(mmap, munmap, 10);
    l.AddInterval(1<<20, 1<<30, PageSize::HUGE_2MB);
    l.AddInterval(1<<10, 1<<20, PageSize::BASE_4KB);
    l.AddInterval(0, 1<<10, PageSize::HUGE_1GB);

    assert(l.At(0)._start_offset == (1<<20));
    assert(l.At(1)._start_offset == (1<<10));
    assert(l.At(2)._start_offset == (0));

    l.Sort();

    assert(l.At(2)._start_offset == (1<<20));
    assert(l.At(1)._start_offset == (1<<10));
    assert(l.At(0)._start_offset == (0));

    std::cout << "**********   "
        << __FILE__
        << " has succeeded!   **********" << std::endl;

}
