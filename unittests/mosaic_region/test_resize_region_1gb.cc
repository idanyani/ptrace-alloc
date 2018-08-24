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
    for (unsigned int i=0; i<size; i += (2*MB)) {
        assert(ptr[i] == WRITTEN_DATA);
    }
}

int main() {

    MosaicRegion hpbr;
    size_t size = 1*GB;
    hpbr.Initialize(size, 0, size, 0, 0, mmap, munmap);
    void *region_base = hpbr.GetRegionBase();
    hpbr.Resize(0);
    hpbr.Resize(size);
    memset(region_base, WRITTEN_DATA, size);
    
    ValidateData((char*)region_base, size);

    for (unsigned int i=size; i>0; i -= (2*MB)) {
        hpbr.Resize(i);
        ValidateData((char*)region_base, size);
    }

    cout << "**********   "
         << __FILE__
         << " has succeeded!   **********" << endl;
}
