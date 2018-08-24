//
// Created by idanyani on 8/24/18.
//

#ifndef PTRACE_ALLOC_FIRSTFITALLOCATOR_H
#define PTRACE_ALLOC_FIRSTFITALLOCATOR_H

#include <stdio.h>
#include <sys/mman.h>
#ifdef THREAD_SAFETY
#include <mutex>
#endif //THREAD_SAFETY

#define PTR_ADD(a, b) ((void*)((size_t)(a) + (size_t)(b)))
#define PTR_SUB(a, b) ((void*)((size_t)(a) - (size_t)(b)))

typedef void *(*FfaMemoryAllocator)(void *addr, size_t length, int prot,
                                    int flags, int fd, off_t offset);
typedef int (*FfaMemoryDeallocator)(void *addr, size_t length);

class FirstFitAllocator {
public:
    FirstFitAllocator(bool enable_validation = false,
                      bool enable_tracing = false);

    ~FirstFitAllocator();

    void Initialize(unsigned int len, void *start, void *end,
                    FfaMemoryAllocator memory_allocator = mmap,
                    FfaMemoryDeallocator memory_deallocator = munmap);

    void *Allocate(size_t size);

    int Free(void *start, size_t size);

    size_t GetFreeSpace();

    void *GetTopAddress();

    bool IsValidDataStructure();

    bool IsAddressAllocated(void *addr);
    bool Contains(void* addr);

private:
    struct MemoryChunk {
    public:
        void *start;
        void *end;
        int next;
    } MC;

    int FindFreeNode();

    int FindFreeMemoryRegionNode(void *start);

    int FindOccupiedMemoryRegionNode(void *start);

    int MoveNodeFromFeeListToOccupied(int free_node, int prev_free_node);

    int AllocateMemoryRegionNode(int free_node, void *start, size_t size);

    int AddFreedRegionToFreeList(void *start, size_t size);

    void CombineAdjacentFreeNodes();

    int FreeOccupiedRegionNode(int node);

    bool _is_initialized;
    MemoryChunk *_array;
    unsigned int _len;
    void *_start;
    void *_end;
    int _occupied_head;
    int _free_head;
    FfaMemoryAllocator _memory_allocator;
    FfaMemoryDeallocator _memory_deallocator;

#ifdef THREAD_SAFETY
    std::mutex _ffa_mutex;
#endif //THREAD_SAFETY

    bool _enable_validation;
    FILE * _log_file;
    bool _enable_tracing;

};


#endif //PTRACE_ALLOC_FIRSTFITALLOCATOR_H
