#include <iostream>
#include <string.h>

#include "base_test_huge_page_backed_region.h"

using namespace std;

int main() {

    test_huge_page_backed_region(2ULL * GB, 100 * MB, (100 * MB + 1 * GB), 0,
                                 0);
/*
    size_t size = 2ULL * GB;
    off_t start_1gb = 100*MB;
    off_t end_1gb = start_1gb + 1*GB;
    off_t start_2mb = 0;
    off_t end_2mb = 0;
    MosaicRegion hpbr(size, start_1gb, end_1gb, start_2mb, end_2mb);
    void* region_base = hpbr.GetRegionBase();
    hpbr.Resize(0);
    hpbr.Resize(size);
    memset(region_base, -1, size);

    // Check numa maps
    test_numa_maps(region_base, (size_t)start_1gb, PAGE_4KB_SIZE);
    test_numa_maps((void*)((off_t)region_base + start_1gb),
            (size_t)(end_1gb - start_1gb), PAGE_1GB_SIZE);
    test_numa_maps((void*)((off_t)region_base + end_1gb),
            (size_t)(size - end_1gb),
            PAGE_4KB_SIZE);
*/

    cout << "**********   "
         << __FILE__
         << " has succeeded!   **********" << endl;
}
