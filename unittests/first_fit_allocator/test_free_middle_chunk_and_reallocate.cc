#include "base_test_first_fit_allocator.h"
#include "../globals.h"

int main() {
    assert(1);

    FirstFitAllocator ffa(true, false);
    void *const start = (void *) (1ul << 30); // 1GB
    void *const end = (void *) (2ul << 30); // 2GB
    size_t region_size = 1ul << 20; // 1MB
    size_t total_space = (size_t) (PTR_SUB(end, start));
    size_t total_alloc = 0;
    const unsigned int len = (total_space / region_size) * 2;
    ffa.Initialize(len, start, end);

    for (unsigned int i = 0; i < 1024; i++) {
        void *region_start = ffa.Allocate(region_size);
        ASSERT_PTR_EQ(region_start, PTR_ADD(start, i * region_size));
        total_alloc += region_size;
        ASSERT_EQ(ffa.GetFreeSpace(), total_space - total_alloc);
    }
    for (unsigned int i = 0; i < 1024; i++) {
        int status = ffa.Free(PTR_ADD(start, i * region_size), region_size);
        ASSERT_EQ(status, 0);
        total_alloc -= region_size;
        ASSERT_EQ(ffa.GetFreeSpace(), total_space - total_alloc);

        void *region_start = ffa.Allocate(region_size);
        ASSERT_PTR_EQ(region_start, PTR_ADD(start, i * region_size));
        total_alloc += region_size;
        ASSERT_EQ(ffa.GetFreeSpace(), total_space - total_alloc);
    }

    printf("**********   %s has succeeded!   **********\n", __FILE__);
    return 0;
}


