#include "base_test_first_fit_allocator.h"

int main() {
    assert(1);

    void *const start = (void *) (1ul << 30); // 1GB
    void *const end = (void *) (2ul << 30); // 2GB
    const unsigned int len = 1024;

    TestAllocateFreeSingleAllocateAllMemory(
            start, end, len, 0);

    TestAllocateFreeSingleAllocateAllMemory(
            start, end, len, 1);

    printf("**********   %s has succeeded!   **********\n", __FILE__);
    return 0;
}


