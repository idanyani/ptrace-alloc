#include "base_test_first_fit_allocator.h"
#include "../globals.h"
#include "../dlmalloc/mem_alloc_hook.h"

void TestAllocateFreeAllocate(
        void *const start,
        void *const end,
        const size_t base_region_size,
        const unsigned int len,
        const short increasing_size,
        const short reverse_first_alloc,
        const short reverse_free,
        const short reverse_second_alloc,
        const short even_free) {
    FirstFitAllocator ffa(true, false);
    void **ptrs = (void**)malloc(len * sizeof(void *));
    size_t total_space = (size_t) (PTR_SUB(end, start));
    size_t total_alloc = 0;
    size_t mrl_len = (total_space / base_region_size) * 10;

    ffa.Initialize(mrl_len, start, end);

    ASSERT_EQ(ffa.GetFreeSpace(), total_space);

    // Allocate all memory
    for (unsigned int i = 0; i < len; i++) {

        int index = i;
        if (reverse_first_alloc) {
            index = len - i - 1;
        }
        size_t region_size = base_region_size;
        if (increasing_size) {
            region_size = (index+1) * base_region_size;
        }

        void *region_start = ffa.Allocate(region_size);
        ASSERT_PTR_EQ(region_start, PTR_ADD(start, total_alloc));
        ptrs[index] = region_start;
        total_alloc += region_size;
        ASSERT_EQ(ffa.GetFreeSpace(), total_space - total_alloc);
    }

    // Free all memory
    for (unsigned int i = 0; i < len; i++) {

        if (even_free && ((i % 2) == 0)) {
            continue;
        }
        int index = i;
        if (reverse_free) {
            index = len - i - 1;
        }
        size_t region_size = base_region_size;
        if (increasing_size) {
            region_size = (index+1) * base_region_size;
        }

        int res = ffa.Free(ptrs[index], region_size);
        ASSERT_EQ(res, 0);
        ptrs[index] = nullptr;
        total_alloc -= region_size;
        ASSERT_EQ(ffa.GetFreeSpace(), total_space - total_alloc);
    }

    // Reallocate all memory
    for (unsigned int i = 0; i < len; i++) {

        if (even_free && ((i % 2) == 0)) {
            continue;
        }

        int index = i;
        if (reverse_second_alloc) {
            index = len - i - 1;
        }
        size_t region_size = base_region_size;
        if (increasing_size) {
            region_size = (index+1) * base_region_size;
        }

        if (ptrs[index] != nullptr) { //i.e., it was not freed
            continue;
        }

        void *region_start = ffa.Allocate(region_size);
        //ASSERT_PTR_EQ(region_start, start + total_alloc);
        ASSERT_PTR_NEQ(region_start, nullptr);
        ptrs[index] = region_start;
        total_alloc += region_size;
        ASSERT_EQ(ffa.GetFreeSpace(), total_space - total_alloc);

    }
}

void TestAllocateFreeSingleAllocateAllMemory(
        void *const start,
        void *const end,
        const unsigned int len,
        const unsigned short reverse_free) {

    FirstFitAllocator ffa(true, false);

    size_t total_space = (size_t) (PTR_SUB(end, start));
    size_t region_size = total_space / len;
    size_t total_alloc = 0;

    ffa.Initialize(len, start, end);

    ASSERT_EQ(ffa.GetFreeSpace(), total_space);

    // allocate all memory with multiple chunks
    for (unsigned int i = 0; i < len; i++) {
        void *region_start = ffa.Allocate(region_size);
        ASSERT_PTR_EQ(region_start, PTR_ADD(start, total_alloc));
        total_alloc += region_size;
        ASSERT_EQ(ffa.GetFreeSpace(), total_space - total_alloc);
    }

    // free all memory
    for (unsigned int i = 0; i < len; i++) {
        total_alloc -= region_size;

        void *ptr = PTR_ADD(start, i * region_size);
        if (reverse_free) {
            ptr = PTR_ADD(start, total_alloc);
        }

        int res = ffa.Free(ptr, region_size);
        ASSERT_EQ(res, 0);
        ASSERT_EQ(ffa.GetFreeSpace(), total_space - total_alloc);
    }

    ASSERT_EQ(ffa.GetFreeSpace(), total_space);

    // alloc all memory with one chunk
    void *region_start = ffa.Allocate(total_space);
    ASSERT_PTR_NEQ(region_start, nullptr);

}
