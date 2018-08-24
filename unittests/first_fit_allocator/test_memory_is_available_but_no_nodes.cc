#include "base_test_first_fit_allocator.h"
#include "../globals.h"

int main() {
    assert(1);

    FirstFitAllocator ffa(true, false);
    void *const start = (void *) (1ul << 30); // 1GB
    void *const end = (void *) (2ul << 30); // 2GB
    const unsigned int len = 256;
    size_t region_size = ((size_t) (PTR_SUB(end, start)) / (2 * len));
    size_t total_space = (size_t) (PTR_SUB(end, start));
    size_t total_alloc = 0;

    //len+1: the extra 1 is for the free_head)
    ffa.Initialize(len + 1, start, end);

    ASSERT_EQ(ffa.GetFreeSpace(), total_space);

    // Allocate all nodes
    for (unsigned int i = 0; i < len; i++) {
        void *region_start = ffa.Allocate(region_size);
        ASSERT_PTR_EQ(region_start, PTR_ADD(start, total_alloc));
        total_alloc += region_size;
        ASSERT_EQ(ffa.GetFreeSpace(), total_space - total_alloc);
    }

    // free all memory
    for (unsigned int i = 0; i < len; i++) {
        total_alloc -= region_size;
        int res = ffa.Free(PTR_ADD(start, total_alloc), region_size);
        ASSERT_EQ(res, 0);
        ASSERT_EQ(ffa.GetFreeSpace(), total_space - total_alloc);
    }

    ASSERT_EQ(ffa.GetFreeSpace(), total_space);

    // Reallocate all nodes
    for (unsigned int i = 0; i < len; i++) {
        void *region_start = ffa.Allocate(region_size);
        ASSERT_PTR_EQ(region_start, PTR_ADD(start, total_alloc));
        total_alloc += region_size;
        ASSERT_EQ(ffa.GetFreeSpace(), total_space - total_alloc);
    }

    ASSERT_NEQ(ffa.GetFreeSpace(), 0);

    void *region_start = ffa.Allocate(1);
    ASSERT_PTR_EQ(region_start, nullptr);

    printf("**********   %s has succeeded!   **********\n", __FILE__);
    return 0;
}


