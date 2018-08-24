//
// Created by idanyani on 8/24/18.
//

#ifndef PTRACE_ALLOC_MEMORYINTERVALLIST_H
#define PTRACE_ALLOC_MEMORYINTERVALLIST_H

#include <sys/types.h>
#include "Globals.h"

class MemoryInterval {
public:
    MemoryInterval() {}
    ~MemoryInterval() {}

    off_t _start_offset;
    off_t _end_offset;
    PageSize _page_size;

    static bool LessThan(const MemoryInterval &lhs,
                         const MemoryInterval &rhs) {
        return lhs._start_offset < rhs._start_offset;
    }
};

typedef void* (*MmapFuncPtr)(void *, size_t, int, int, int, off_t);
typedef int (*MunmapFuncPtr)(void *, size_t);

class MemoryIntervalList {
public:
    MemoryIntervalList();
    void Initialize(MmapFuncPtr allocator,
                    MunmapFuncPtr deallocator,
                    size_t capcaity);
    ~MemoryIntervalList();

    size_t GetLength();
    MemoryInterval& At(int i);

    void AddInterval(off_t start_offset,
                     off_t end_offset,
                     PageSize page_size);
    void Sort();

private:
    void SwapIntetrvals(int i, int j);
    void* AllocateMemory(size_t size);
    int FreeMemory(void* addr, size_t size);

    MemoryInterval* _interval_list;
    size_t _list_capcaity;
    size_t _list_length;
    MmapFuncPtr _mmap;
    MunmapFuncPtr _munmap;
};


#endif //PTRACE_ALLOC_MEMORYINTERVALLIST_H
