//
// Created by idanyani on 8/24/18.
//

#include "FirstFitAllocator.h"
#include <assert.h>

#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <unistd.h>

#include <sstream>
#include <iostream>
#include <fstream>

// TODO:
// 1) defragmentation of memory region list
//    i.e., check if there are two free nodes that can be combined to one
//    contigious node
// 2) freeing partial region, i.e., start_region < free_ptr < end_region

#ifdef THREAD_SAFETY
#define MUTEX_GUARD(lock) std::lock_guard<std::mutex> guard(lock)
#else //THREAD_SAFETY
#define MUTEX_GUARD(lock)
#endif //THREAD_SAFETY

#define TRACE(f_, ...) {    \
    if (_enable_tracing) {      \
        fprintf(_log_file, (f_), __VA_ARGS__);   \
        fflush(_log_file);      \
}}

#define RUN_VALIDATION() {              \
    if (_enable_validation) {           \
        assert(IsValidDataStructure()); \
}}

void FirstFitAllocator::Initialize(unsigned int len, void *start, void *end,
                                   FfaMemoryAllocator memory_allocator,
                                   FfaMemoryDeallocator memory_deallocator) {
    MUTEX_GUARD(_ffa_mutex);

    TRACE("Initialize - len: %u , start: %p , end: %p\n", len, start, end);

    assert(_is_initialized == false);
    _len = len;
    _start = start;
    _end = end;
    _memory_allocator = memory_allocator;
    _memory_deallocator = memory_deallocator;

    size_t aligned_array_size = len * sizeof(MC);
    aligned_array_size = (4096 - (aligned_array_size % 4096))
                         + aligned_array_size;

    _array = static_cast<MemoryChunk*>(
            memory_allocator(NULL,
                             aligned_array_size,
                             PROT_READ|PROT_WRITE,
                             MAP_PRIVATE|MAP_ANONYMOUS,
                             -1, 0));

    for (unsigned int i = 0; i < len; i++) {
        _array[i].start = NULL;
        _array[i].end = NULL;
        _array[i].next = -1;
    }
    _occupied_head = -1;
    _free_head = 0;
    _array[_free_head].start = start;
    _array[_free_head].end = end;
    _array[_free_head].next = -1;

    _is_initialized = true;

    RUN_VALIDATION();
}

int FirstFitAllocator::FindFreeNode() {
    assert(_is_initialized == true);
    for (unsigned int i = 0; i < _len; i++) {
        if (_array[i].start == NULL) {
            return i;
        }
    }
    return -1;
}


int FirstFitAllocator::FindOccupiedMemoryRegionNode(void *start) {
    assert(_is_initialized == true);
    for (int i = _occupied_head;
         i >= 0;
         i = _array[i].next) {
        if (start == _array[i].start ||
            (start >= _array[i].start && start < _array[i].end)) {
            return i;
        }
    }
    return -1;
}

int FirstFitAllocator::FindFreeMemoryRegionNode(void *start) {
    assert(_is_initialized == true);
    for (int i = _free_head;
         i >= 0;
         i = _array[i].next) {
        if (start == _array[i].start ||
            (start >= _array[i].start && start < _array[i].end)) {
            return i;
        }
    }
    return -1;
}


int FirstFitAllocator::AllocateMemoryRegionNode(int free_node,
                                                void *start,
                                                size_t size) {
    assert(_is_initialized == true);

    // find a free node to store the new allocated memory region
    if (free_node == -1) {
        free_node = FindFreeNode();
    }
    // check if the list is already full
    if (free_node == -1) {
        return -1;
    }
    if (free_node == _free_head) {
        _free_head = _array[_free_head].next;
    }

    int i = -1, prev_i = -1;
    // find where to add the new allocated memory region
    // (while keeping the list ordered by start addresses)
    for (i = _occupied_head;
         i >= 0;
         prev_i = i, i = _array[i].next) {
        if (_array[i].start > start) {
            break;
        }
    }

    // if occupied_head is not initialized i.e. this is the first allocation
    // or it should place before the current occupied_head
    if (prev_i == -1) {
        _occupied_head = free_node;
        _array[free_node].next = i;
    }
        // if the new allocated memory region is the last
        // node in the occupied list
    else if (i == -1) {
        _array[prev_i].next = free_node;
        _array[free_node].next = -1;
    } else {
        _array[prev_i].next = free_node;
        _array[free_node].next = i;
    }

    _array[free_node].start = start;
    _array[free_node].end = PTR_ADD(start, size);

    return free_node;
}

int FirstFitAllocator::MoveNodeFromFeeListToOccupied(int free_node, int prev_free_node) {
    // save next of free_node to update the free list later on
    // for not loosing free_node.next because it will be updated
    // inside AllocateMemoryRegionNode
    int next_free_node = _array[free_node].next;
    // update the occupied list indices
    void *start = _array[free_node].start;
    void *end = _array[free_node].end;
    size_t size = (size_t)(PTR_SUB(end, start));
    int node = AllocateMemoryRegionNode(free_node, start, size);
    if (node < 0) {
        return node;
    }
    // If the node is the last node in the free list
    if (free_node == -1) {
        _array[prev_free_node].next = -1;
    } else if (prev_free_node == -1) { // free_node is the _free_head
        // do nothing sinze it was handled inside AllocateMemoryRegionNode
    } else { // 3) Otherwise
        _array[prev_free_node].next = next_free_node;
    }
    return node;
}

void *FirstFitAllocator::Allocate(size_t size) {
    MUTEX_GUARD(_ffa_mutex);

    TRACE("Allocate - size: %lu --> ", size);

    assert(_is_initialized == true);

    if (size == 0) {
        return NULL;
    }
    // Check if there still available room in the memory region list
    // for the new region
    if (_free_head == -1) {
        return NULL;
    }

    void *res = NULL;

    // find the first fit free node
    int prev_i = -1;
    for (int i = _free_head;
         i >= 0;
         prev_i = i, i = _array[i].next) {
        size_t slot_size = (size_t) (PTR_SUB(_array[i].end, _array[i].start));
        if (slot_size >= size) {
            res = _array[i].start;
            int node = -1;
            // to save list nodes, if current node has exactly the same
            // size as the required region to allocate then move it from
            // free list to occupied list
            if (slot_size == size) {
                node = MoveNodeFromFeeListToOccupied(i, prev_i);
            } else { // Otherwise, allocate new node
                node = AllocateMemoryRegionNode(-1, res, size);
                if (node >= 0)
                    _array[i].start = PTR_ADD(_array[i].start, size);
            }
            if (node < 0) {
                res = NULL;
            }
            TRACE("%p\n", res);
            break;
        }
    }
    RUN_VALIDATION();
    return res;
}

int FirstFitAllocator::AddFreedRegionToFreeList(void *start, size_t size) {
    assert(_is_initialized == true);
    // find a region that could be combined with the freed one
    int i = -1, prev_i = -1;
    for (i = _free_head;
         i >= 0;
         prev_i = i, i = _array[i].next) {
        if (PTR_ADD(start, size) == _array[i].start) {
            _array[i].start = start;
            return 0;
        }
        if (_array[i].end == start) {
            _array[i].end = PTR_ADD(start, size);
            return 0;
        }
    }
    // Could not find contigious free region to append
    // this region to it
    // try to find new node to allocate in the free list
    int node = FindFreeNode();
    if (node < 0) {
        return node;
    }

    prev_i = -1;
    // find where to add the freed memory region
    // (while keeping the list ordered by start addresses)
    for (i = _free_head;
         i >= 0;
         prev_i = i, i = _array[i].next) {
        if (_array[i].start > start) {
            break;
        }
    }
    // if the freed node should be placed before the free_head
    if (prev_i == -1) {
        _array[node].next = _free_head;
        _free_head = node;
        //_array[node].next = i;
    }
        // if the freed node is the last
        // node in the free list
    else if (i == -1) {
        _array[prev_i].next = node;
        _array[node].next = -1;
    } else {
        _array[prev_i].next = node;
        _array[node].next = i;
    }

    _array[node].start = start;
    _array[node].end = PTR_ADD(start, size);

    CombineAdjacentFreeNodes();

    return 0;
}

void FirstFitAllocator::CombineAdjacentFreeNodes() {
    assert(_is_initialized == true);
    int i = -1, prev_i = -1;
    // find if there are two adjacent free nodes that could
    // be merged.
    // We assume that the list is sorted (add and remove nodes
    // from the free list are done in sorted order).
    for (i = _free_head;
         i >= 0;
         prev_i = i, i = _array[i].next) {
        if (prev_i == -1) {
            continue;
        }
        if (_array[prev_i].end == _array[i].start) {
            // combine the adjacent nodes
            _array[prev_i].end = _array[i].end;
            // remove current node
            _array[prev_i].next = _array[i].next;
            _array[i].start = _array[i].end = NULL;
            _array[i].next = -1;
            // dont move to next node because current one
            // may could be merged with next one
            i = prev_i;
        }
    }

}

int FirstFitAllocator::FreeOccupiedRegionNode(int node) {
    assert(_is_initialized == true);
    int i = -1, prev_i = -1;
    for (i = _occupied_head;
         i >= 0;
         prev_i = i, i = _array[i].next) {
        if (i == node) {
            break;
        }
    }
    if (i == -1) {
        return -1;
    }
    // the node to be freed is the occupied_head
    if (prev_i == -1) {
        _occupied_head = _array[i].next;
    } else {
        _array[prev_i].next = _array[i].next;
    }
    _array[i].start = _array[i].end = NULL;
    _array[i].next = -1;

    return 0;
}

int FirstFitAllocator::Free(void *start, size_t size) {
    MUTEX_GUARD(_ffa_mutex);

    int res = -100;

    TRACE("Free - start: %p , size: %lu\n", start, size);

    assert(_is_initialized == true);
    int node = FindOccupiedMemoryRegionNode(start);
    if (node < 0) {
        return node;
    }
    size_t node_size = (size_t) (PTR_SUB(_array[node].end, _array[node].start));
    if (node_size < size) {
        fprintf(stderr, "FirstFitAllocator::Free - [Error]: missmatch sizes\n");
        fprintf(stderr, "\tFree(%p) - node_size: %lu , free_size: %lu\n", start, node_size, size);
        return -2;
    }
    if (node_size == size) {
        res = FreeOccupiedRegionNode(node);
        if (res < 0) {
            RUN_VALIDATION();
            return res;
        }
    } else {
        // TODO: handle freeing memory region from the middle, i.e.,
        // region_start < free_ptr < region_end
        _array[node].start = PTR_ADD(_array[node].start, size);
    }
    res = AddFreedRegionToFreeList(start, size);
    RUN_VALIDATION();
    return res;
}

FirstFitAllocator::FirstFitAllocator(bool enable_validation,
                                     bool enable_tracing)
        : _is_initialized(false),
          _enable_validation(enable_validation),
          _enable_tracing(enable_tracing) {

    static int logger_index = 0;
    _log_file = NULL;
    if (enable_tracing) {
        logger_index++;
        std::string pid_str;
        std::stringstream out;
        out << getpid();
        pid_str = out.str();

        std::string fileName = "ffa_trace." + pid_str + ".out" + std::to_string(logger_index);
        //_trace_file.open(fileName.c_str());
        _log_file = fopen (fileName.c_str(), "w+");
    }
}

FirstFitAllocator::~FirstFitAllocator() {
    _is_initialized = false;

    if (_enable_tracing && _log_file) {
        fclose(_log_file);
    }

    size_t aligned_array_size = _len * sizeof(MC);
    aligned_array_size = (4096 - (aligned_array_size % 4096))
                         + aligned_array_size;

    _memory_deallocator(_array, aligned_array_size);
    _array = NULL;
}

size_t FirstFitAllocator::GetFreeSpace() {
    MUTEX_GUARD(_ffa_mutex);

    assert(_is_initialized == true);
    size_t sum = 0;
    for (int i = _free_head;
         i >= 0;
         i = _array[i].next) {
        sum += (size_t) (PTR_SUB(_array[i].end, _array[i].start));
    }
    return sum;
}

void *FirstFitAllocator::GetTopAddress() {
    MUTEX_GUARD(_ffa_mutex);

    assert(_is_initialized == true);
    void* top_addr = _start;
    for (int i = _occupied_head;
         i >= 0;
         i = _array[i].next) {
        if (_array[i].end > top_addr) {
            top_addr = _array[i].end;
        }
    }
    return top_addr;
}

bool FirstFitAllocator::Contains(void *addr) {
    assert(_is_initialized == true);
    return (addr >= _start && addr < _end);
}

bool FirstFitAllocator::IsAddressAllocated(void *addr) {
    MUTEX_GUARD(_ffa_mutex);

    assert(_is_initialized == true);
    return (addr >= _start && addr < _end);
    int node = FindOccupiedMemoryRegionNode(addr);
    if (node < 0) {
        return false;
    }
    return true;
}

bool FirstFitAllocator::IsValidDataStructure() {
    // 1) Validate no overlapping between nodes
    int overlap_i = -1, overlap_j = -1;
    for (int i = _occupied_head; i >= 0; i = _array[i].next) {
        for (int j = _occupied_head; j >= 0; j = _array[j].next) {
            if (i == j)
                continue;
            if ((_array[i].start >= _array[j].start &&
                 _array[i].start < _array[j].end)
                ||
                (_array[j].start >= _array[i].start &&
                 _array[j].start < _array[i].end)) {
                overlap_i = i;
                overlap_j = j;
            }
        }
        for (int j = _free_head; j >= 0; j = _array[j].next) {
            if ((_array[i].start >= _array[j].start &&
                 _array[i].start < _array[j].end)
                ||
                (_array[j].start >= _array[i].start &&
                 _array[j].start < _array[i].end)) {
                overlap_i = i;
                overlap_j = j;
            }
        }
    }
    for (int i = _free_head; i >= 0; i = _array[i].next) {
        for (int j = _free_head; j >= 0; j = _array[j].next) {
            if (i == j)
                continue;
            if ((_array[i].start >= _array[j].start &&
                 _array[i].start < _array[j].end)
                ||
                (_array[j].start >= _array[i].start &&
                 _array[j].start < _array[i].end)) {
                overlap_i = i;
                overlap_j = j;
            }
        }
        for (int j = _occupied_head; j >= 0; j = _array[j].next) {
            if ((_array[i].start >= _array[j].start &&
                 _array[i].start < _array[j].end)
                ||
                (_array[j].start >= _array[i].start &&
                 _array[j].start < _array[i].end)) {
                overlap_i = i;
                overlap_j = j;
            }
        }
    }
    if (overlap_i != -1 || overlap_j != -1) {
        fprintf(stderr, "FirstFitAllocator validation process failed with overlapping:\n");
        fprintf(stderr, "\tnode %d : [%p - %p]\n",
                overlap_i, _array[overlap_i].start, _array[overlap_i].end);
        fprintf(stderr, "\tnode %d : [%p - %p]\n",
                overlap_j, _array[overlap_j].start, _array[overlap_j].end);
        return false;
    }
    // 2) Validate total size of all nodes (occupied and free) is equal
    // to original size
    size_t expected_size = (size_t)PTR_SUB(_end, _start);
    size_t total_size = 0;
    for (int i = _occupied_head; i >= 0; i = _array[i].next) {
        total_size += (size_t)PTR_SUB(_array[i].end, _array[i].start);
    }
    for (int i = _free_head; i >= 0; i = _array[i].next) {
        total_size += (size_t)PTR_SUB(_array[i].end, _array[i].start);
    }
    if (total_size != expected_size) {
        fprintf(stderr, "FirstFitAllocator validation process failed with missmatch total size:\n");
        fprintf(stderr, "\ttotal-size: %lu , expected-size: %lu\n", total_size, expected_size);
        return false;
    }

    /*
    // 3) Validate there are no disconnected nodes
    for (unsigned int i = 0; i < _len; i++) {
    if (_array[i].start == NULL) {
    continue;
    }
    int free_node = FindFreeMemoryRegionNode(_array[i].start);
    int occupied_node = FindOccupiedMemoryRegionNode(_array[i].start);
    if (free_node < 0 && occupied_node < 0) {
    fprintf(stderr, "FirstFitAllocator::Validate() failed with disconnected node:\n");
    fprintf(stderr, "\tnode %d : [%p - %p]\n", i, _array[i].start, _array[i].end);
    return false;
    }
    }
    */
    return true;
}
