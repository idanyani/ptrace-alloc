#include "base_test_first_fit_allocator.h"

void TestAllocateFreeAllocateIncreasingRegionSize(
        void *const start,
        void *const end,
        const unsigned int len) {
    size_t total_space = (size_t) (PTR_SUB(end, start));
    TestAllocateFreeAllocate(
            start,
            end,
            (total_space / len) / len,
            len,
            1,
            0, 0, 0, 0);
}


int main() {
    assert(1);

    void *const start = (void *) (1ul << 30); // 1GB
    void *const end = (void *) (2ul << 30); // 2GB
    TestAllocateFreeAllocateIncreasingRegionSize(
            start,
            end,
            1);
    TestAllocateFreeAllocateIncreasingRegionSize(
            start,
            end,
            2);
    TestAllocateFreeAllocateIncreasingRegionSize(
            start,
            end,
            256);

    printf("**********   %s has succeeded!   **********\n", __FILE__);
    return 0;
}


