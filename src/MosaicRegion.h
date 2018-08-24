//
// Created by idanyani on 8/24/18.
//

#ifndef PTRACE_ALLOC_MOSAICREGION_H
#define PTRACE_ALLOC_MOSAICREGION_H

#include <cstddef>
#include <sys/types.h>
#include <vector>
#include "Globals.h"
#include "MemoryIntervalList.h"

class MosaicRegion {
public:
    void Initialize(size_t region_size,
                    off_t start_1gb_offset,
                    off_t end_1gb_offset,
                    off_t start_2mb_offset,
                    off_t end_2mb_offset,
                    MmapFuncPtr allocator,
                    MunmapFuncPtr deallocator);

    MosaicRegion();
    ~MosaicRegion();

    int Resize(size_t new_size);

    void *GetRegionBase();

    size_t GetRegionSize();

    size_t GetRegionMaxSize();

private:
    size_t ExtendRegion(size_t new_size);

    size_t ShrinkRegion(size_t new_size);

    void *AllocateMemory(void *start_address, size_t len, PageSize page_size);

    void DeallocateMemory(void *addr, size_t len);

    void* RegionIntervalListMemAlloc(size_t s);

    int RegionIntervalListMemDealloc(void* addr, size_t s);

    void *_region_start;
    size_t _region_max_size;
    MemoryIntervalList _region_intervals;
    size_t _region_current_size;
    bool _initialized;

    MmapFuncPtr _memory_allocator;
    MunmapFuncPtr _memory_deallocator;
};


#endif //PTRACE_ALLOC_MOSAICREGION_H
