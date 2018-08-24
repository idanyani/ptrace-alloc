#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

#include <iostream>
#include <string.h>

#include "base_test_huge_page_backed_region.h"

#define WRITTEN_DATA (0x34)

using namespace std;

void ValidateData(char *ptr, size_t size) {
    for (unsigned int i=0; i<size; i += (MB)) {
        assert(ptr[i] == WRITTEN_DATA);
    }
}

int main() {

    MosaicRegion hpbr;
    size_t size = 10*MB;
    size_t start_2mb = 3*MB;
    size_t end_2mb = 5*MB;
    hpbr.Initialize(size, 0, 0, start_2mb, end_2mb, mmap, munmap);
    void *region_base = hpbr.GetRegionBase();

    for (unsigned int i=0; i<=size; i += (MB)) {
        hpbr.Resize(i);
        memset(region_base, WRITTEN_DATA, i);
        ValidateData((char*)region_base, i);
    }
    
    for (unsigned int i=0; i<=size; i += (MB)) {
        hpbr.Resize(size-i);
        ValidateData((char*)region_base, size-i);
    }

    cout << "**********   "
         << __FILE__
         << " has succeeded!   **********" << endl;
}

