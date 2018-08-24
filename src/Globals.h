//
// Created by idanyani on 8/24/18.
//

#ifndef PTRACE_ALLOC_GLOBALS_H
#define PTRACE_ALLOC_GLOBALS_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define ASSERT_PTR_EQ(a, b) { \
    if ((a) != (b)) { \
        printf("%p != %p\n", (a), (b)); \
        assert((a) == (b)); \
    }}

#define ASSERT_PTR_NEQ(a, b) { \
    if ((a) == (b)) { \
        printf("%p == %p\n", (a), (b)); \
        assert((a) != (b)); \
    }}


#define ASSERT_EQ(a, b) { \
    if ((a) != (b)) { \
        printf("%ld != %ld\n", (long int)(a), (long int)(b)); \
        assert((a) == (b)); \
    }}

#define ASSERT_NEQ(a, b) { \
    if ((a) == (b)) { \
        printf("%ld == %ld\n", (long int)(a), (long int)(b)); \
        assert((a) != (b)); \
    }}

#define ROUND_UP(num, mul) \
    ((((size_t)(num) + (size_t)(mul) - 1) / (size_t)(mul)) * (size_t)(mul))
#define ROUND_DOWN(num, mul) \
    ((((size_t)(num)) / ((size_t)(mul))) * ((size_t)(mul)))
#define IS_ALIGNED(num, alignment) (((size_t)(num) % (size_t)(alignment)) == 0)

#define __WRITE_ERROR_AND_EXIT(msg)    \
{\
    write(STDERR_FILENO, __FILE__, strlen(__FILE__)); \
    write(STDERR_FILENO, ":", 1); \
    write(STDERR_FILENO, __FUNCTION__, strlen(__FUNCTION__)); \
    write(STDERR_FILENO, "\n", 1); \
    char msg_buf[] = msg; \
    write(STDERR_FILENO, msg_buf, sizeof(msg_buf)); \
    write(STDERR_FILENO, "\n", 1); \
    sync(); \
    _exit(1); \
}

/* use write and sync for throwing exceptions because they are not allocating
 * new memory or calling memory allocations APIs like other printing APIs or
 * c++ std exceptions.
 * Using or calling memory allocation APIs during thrwoing exceptions causes
 * infinite recursive calls to malloc (because these excpetions will be
 * thrown from malloc, or other allocation APIs)
*/
#define THROW_EXCEPTION(msg)    \
{\
    char prefix_buf[] = "Exception thrown at: "; \
    write(STDERR_FILENO, prefix_buf, sizeof(prefix_buf)); \
    __WRITE_ERROR_AND_EXIT(msg) \
}

#define ASSERT(cond) \
{\
    if (!(cond)) { \
        __WRITE_ERROR_AND_EXIT("Code assertion!") \
}}

/**********************************************
* mmap special flags for using huge pages
**********************************************/
#define MMAP_PROTECTION (PROT_READ | PROT_WRITE)
#define MMAP_FLAGS (MAP_PRIVATE | MAP_ANONYMOUS)
#define MAP_HUGE_2MB    (21 << MAP_HUGE_SHIFT)
#define MAP_HUGE_1GB    (30 << MAP_HUGE_SHIFT)

enum class PageSize : size_t {
    BASE_4KB = 4096, // (1 << 30)
    HUGE_2MB = 2097152, // (1 << 21)
    HUGE_1GB = 1073741824, // (1 << 12)
    UNKNOWN = 0
};

#endif //PTRACE_ALLOC_GLOBALS_H
