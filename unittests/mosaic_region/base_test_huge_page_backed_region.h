#ifndef _BASE_TEST_HUGE_PAGE_BACKED_REGION_H_
#define _BASE_TEST_HUGE_PAGE_BACKED_REGION_H_

#include <assert.h>
#include <stdio.h>

#include "MosaicRegion.h"
#include "../numa_maps/numa_maps.h"

#define GB (1073741824)
#define MB (1048576)

void test_numa_maps(void *start_addr, size_t size, size_t page_size);

void test_huge_page_backed_region(size_t size,
                                  off_t start_1gb,
                                  off_t end_1gb,
                                  off_t start_2mb,
                                  off_t end_2mb);

#endif //_BASE_TEST_HUGE_PAGE_BACKED_REGION_H_
