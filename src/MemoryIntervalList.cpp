//
// Created by idanyani on 8/24/18.
//

#include "MemoryIntervalList.h"
#include <system_error>
#include <sys/mman.h>

MemoryIntervalList::MemoryIntervalList() :
        _list_capcaity(0),
        _list_length(0) {
}

void *MemoryIntervalList::AllocateMemory(size_t size) {
    size_t length = ROUND_UP(size, PageSize::BASE_4KB);
    return _mmap(NULL, length, MMAP_PROTECTION, MMAP_FLAGS, -1, 0);
}

int MemoryIntervalList::FreeMemory(void* addr, size_t size) {
    size_t length = ROUND_UP(size, PageSize::BASE_4KB);
    return _munmap(addr, length);
}

void MemoryIntervalList::Initialize(MmapFuncPtr allocator,
                                    MunmapFuncPtr deallocator,
                                    size_t capcaity) {
    _list_capcaity = capcaity;
    _list_length = 0;
    _mmap = allocator;
    _munmap = deallocator;
    _interval_list = static_cast<MemoryInterval*>(
            AllocateMemory(capcaity * sizeof (MemoryInterval)));
    if (_interval_list == nullptr) {
        std::error_code ec(errno, std::generic_category());
        THROW_EXCEPTION("Failed to allocate Memory Region Interval List");
    }
}

MemoryIntervalList::~MemoryIntervalList() {
    int res = FreeMemory(_interval_list, _list_capcaity);
    if (res != 0) {
        THROW_EXCEPTION("Failed to deallocate Memory Region Interval List");
    }
}

void MemoryIntervalList::AddInterval(
        off_t start_offset, off_t end_offset,
        PageSize page_size) {
    if (_list_length == _list_capcaity) {
        THROW_EXCEPTION("Memory Region Interval List is already full");
    }
    _interval_list[_list_length]._start_offset = start_offset;
    _interval_list[_list_length]._end_offset = end_offset;
    _interval_list[_list_length]._page_size = page_size;
    _list_length++;
}

void MemoryIntervalList::SwapIntetrvals(int i, int j) {
    // do the swap field by fierld to prevent calling copy-ctor
    auto start_offset = _interval_list[i]._start_offset;
    _interval_list[i]._start_offset = _interval_list[j]._start_offset;
    _interval_list[j]._start_offset = start_offset;

    auto end_offset = _interval_list[i]._end_offset;
    _interval_list[i]._end_offset = _interval_list[j]._end_offset;
    _interval_list[j]._end_offset = end_offset;

    auto page_size = _interval_list[i]._page_size;
    _interval_list[i]._page_size = _interval_list[j]._page_size;
    _interval_list[j]._page_size = page_size;
}

void MemoryIntervalList::Sort() {
    // Min/Max Sort
    for (unsigned int i=0; i<_list_length; i++) {
        for (unsigned int j=i; j<_list_length; j++) {
            if (!MemoryInterval::LessThan(_interval_list[i],
                                          _interval_list[j])) {
                SwapIntetrvals(i, j);
            }
        }
    }
}

size_t MemoryIntervalList::GetLength()
{
    return _list_length;
}

MemoryInterval& MemoryIntervalList::At(int i)
{
    return _interval_list[i];
}
