#include "base_test_first_fit_allocator.h"

void TestAllocateReverseFreeAllocateAllMemory(
        void *const start,
        void *const end,
        const size_t region_size,
        const unsigned int len) {
    TestAllocateFreeAllocate(
            start,
            end,
            region_size,
            len,
            0, 0, 1, 0, 0);
}


int main() {
    assert(1);

    void *const start = (void *) (1ul << 30); // 1GB
    void *const end = (void *) (2ul << 30); // 2GB
    const unsigned int len = 256;
    size_t region_size = ((size_t) (PTR_SUB(end, start)) / len);
    TestAllocateReverseFreeAllocateAllMemory(
            start,
            end,
            region_size,
            1);
    TestAllocateReverseFreeAllocateAllMemory(
            start,
            end,
            region_size,
            2);
    TestAllocateReverseFreeAllocateAllMemory(
            start,
            end,
            region_size,
            len);

    printf("**********   %s has succeeded!   **********\n", __FILE__);
    return 0;
}

