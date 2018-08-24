#include "base_test_first_fit_allocator.h"

void TestAllocateFreeEvenChunksAllocateAllMemory(
        void *const start,
        void *const end,
        const size_t region_size,
        const unsigned int len) {
    TestAllocateFreeAllocate(
            start,
            end,
            region_size,
            len,
            0, 0, 0, 0, 1);
}

int main() {
    assert(1);

    void *const start = (void *) (1ul << 30); // 1GB
    void *const end = (void *) (2ul << 30); // 2GB
    const unsigned int len = 256;
    size_t region_size = ((size_t) (PTR_SUB(end, start)) / len);

    TestAllocateFreeAllocate(
            start,
            end,
            region_size,
            len,
            0, 0, 0, 0, 1);

    TestAllocateFreeAllocate(
            start,
            end,
            region_size,
            len,
            0, 0, 1, 0, 1);

    TestAllocateFreeAllocate(
            start,
            end,
            region_size,
            len,
            0, 0, 0, 1, 1);

    TestAllocateFreeAllocate(
            start,
            end,
            region_size,
            len,
            0, 0, 1, 1, 1);

    // The following test is to cover the broken-free-list bug:
    // the bug is about breaking free list and lossing the last
    // node in free-list which contains the largest free memory
    // region, and that occured when first and one of last allocated
    // chunks were freed, then the free-list will be broken.
    size_t total_space = (size_t) (PTR_SUB(end, start));
    TestAllocateFreeAllocate(
            start,
            end,
            (total_space / len) / len,
            len,
            1, 0, 1, 1, 1);

    printf("**********   %s has succeeded!   **********\n", __FILE__);
    return 0;
}


