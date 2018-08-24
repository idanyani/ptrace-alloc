#ifndef TEST_FIRST_FIT_ALLOCATOR_H_
#define TEST_FIRST_FIT_ALLOCATOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "first_fit_allocator.h"

void *MyAllocateArrayOfMrn(unsigned int len);

int MyFreeArrayOfMrn(void *array, unsigned int len);

void TestAllocateFreeAllocate(
        void *const start,
        void *const end,
        const size_t base_region_size,
        const unsigned int len,
        const short increasing_size,
        const short reverse_first_alloc,
        const short reverse_free,
        const short reverse_second_alloc,
        const short even_free);

void TestAllocateFreeSingleAllocateAllMemory(
        void *const start,
        void *const end,
        const unsigned int len,
        const unsigned short reverse_free);

#endif //TEST_FIRST_FIT_ALLOCATOR_H_
