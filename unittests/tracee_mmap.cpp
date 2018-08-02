#include <iostream>

#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>


int main() {
    if (getpid() == 0) return 1;// this is to prevent optimizing out getpid call

    intptr_t res = (intptr_t) mmap(NULL,
                                   4,
                                   PROT_READ | PROT_WRITE,
                                   MAP_PRIVATE | MAP_ANONYMOUS,
                                   -1,
                                   0);
    return res;
}
