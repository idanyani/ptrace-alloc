#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>


int main() {
    if (kill(getpid(), 0) != 0) return -1;

    intptr_t res = (intptr_t) mmap(NULL,
                                   4,
                                   PROT_READ | PROT_WRITE,
                                   MAP_PRIVATE | MAP_ANONYMOUS,
                                   -1,
                                   0);
    return (res == getpid()) ? 0 : -1;
}
