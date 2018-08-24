#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "base_test_huge_page_backed_region.h"

void test_numa_maps(void *start_addr, size_t size, size_t page_size) {
    NumaMaps numa_maps(getpid());
    ASSERT_EQ(numa_maps.GetMemoryRange(start_addr)._total_size, size);
    auto memory_range = numa_maps.GetMemoryRange(start_addr);
    ASSERT_EQ(memory_range._total_size, size);
    ASSERT_EQ(static_cast<size_t>(memory_range._page_size), page_size);
    ASSERT_EQ(memory_range._type, NumaMaps::MemoryRange::Type::ANONYMOUS);
    ASSERT_EQ(memory_range._total_pages, (size / page_size));
}

void test_huge_page_backed_region(size_t size,
                                  off_t start_1gb,
                                  off_t end_1gb,
                                  off_t start_2mb,
                                  off_t end_2mb) {
    MosaicRegion hpbr;
    hpbr.Initialize(size, start_1gb, end_1gb, start_2mb, end_2mb, mmap, munmap);
    void *region_base = hpbr.GetRegionBase();
    hpbr.Resize(0);
    hpbr.Resize(size);
    memset(region_base, -1, size);

    off_t start_first_huge_page = start_1gb;
    off_t end_first_huge_page = end_1gb;
    off_t start_second_huge_page = start_2mb;
    off_t end_second_huge_page = end_2mb;
    size_t page_size_first = static_cast<size_t>(PageSize::HUGE_1GB);
    size_t page_size_second = static_cast<size_t>(PageSize::HUGE_2MB);
    if (start_2mb < start_first_huge_page) {
        start_first_huge_page = start_2mb;
        end_first_huge_page = end_2mb;
        start_second_huge_page = start_1gb;
        end_second_huge_page = end_1gb;
        page_size_first = static_cast<size_t>(PageSize::HUGE_2MB);
        page_size_second = static_cast<size_t>(PageSize::HUGE_1GB);
    }

    off_t start_current = 0;
    if ((end_first_huge_page - start_first_huge_page) > 0) {
        if (start_current < start_first_huge_page) {
            test_numa_maps((void *) ((off_t) region_base + start_current),
                           (size_t) start_first_huge_page,
                           static_cast<size_t>(PageSize::BASE_4KB));
            start_current = start_first_huge_page;
        }
        test_numa_maps((void *) ((off_t) region_base + start_current),
                       (size_t) (end_first_huge_page - start_first_huge_page),
                       page_size_first);
        start_current = end_first_huge_page;
    }
    if ((end_second_huge_page - start_second_huge_page) > 0) {
        if (start_current < start_second_huge_page) {
            test_numa_maps((void *) ((off_t) region_base + start_current),
                           (size_t) (start_second_huge_page - start_current),
                           static_cast<size_t>(PageSize::BASE_4KB));
            start_current = start_second_huge_page;
        }
        test_numa_maps((void *) ((off_t) region_base + start_current),
                       (size_t) (end_second_huge_page - start_second_huge_page),
                       page_size_second);
        start_current = end_second_huge_page;
    }
    if (start_current < (off_t) size) {
        test_numa_maps((void *) ((off_t) region_base + start_current),
                       (size_t) (size - start_current),
                       static_cast<size_t>(PageSize::BASE_4KB));
    }
}


