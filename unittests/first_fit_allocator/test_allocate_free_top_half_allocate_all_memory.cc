#include "base_test_first_fit_allocator.h"
#include "../globals.h"

int main() {
    assert(1);

    FirstFitAllocator ffa(true, false);
    const unsigned int len = 1024;
    void *const start = (void *) (1ul << 30); // 1GB
    void *const end = (void *) (2ul << 30); // 2GB
    size_t total_space = (size_t) (PTR_SUB(end, start));
    size_t total_alloc = 0;

    ffa.Initialize(len, start, end);

    size_t region_size = total_space / len;
    void *region_start = nullptr;

    // Allocate all memory
    for (unsigned int i = 0; i < len; i++) {
        region_start = ffa.Allocate(region_size);
        ASSERT_PTR_EQ(region_start, PTR_ADD(start, total_alloc));
        total_alloc += region_size;
        ASSERT_EQ(ffa.GetFreeSpace(), (total_space - total_alloc));
    }

    // Free top half
    for (unsigned int i = len / 2; i < len; i++) {
        int status = ffa.Free(
                PTR_ADD(start, i * region_size),
                region_size);
        ASSERT_EQ(status, 0);
        total_alloc -= region_size;
        ASSERT_EQ(ffa.GetFreeSpace(), (total_space - total_alloc));
    }

    // Reallocate top half
    for (unsigned int i = len / 2; i < len; i++) {
        region_start = ffa.Allocate(region_size);
        ASSERT_PTR_NEQ(region_start, nullptr);
        total_alloc += region_size;
        ASSERT_EQ(ffa.GetFreeSpace(), (total_space - total_alloc));
    }

    printf("**********   %s has succeeded!   **********\n", __FILE__);
    return 0;
}
