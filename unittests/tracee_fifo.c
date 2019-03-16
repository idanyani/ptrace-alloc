//
// Created by mac on 2/22/19.
//
#include <unistd.h>     // getcwd
#include <sys/mman.h>   // mmap
#include <stdio.h>
#include <memory.h>     // strcmp

int main(int argc, char **argv){

    char buffer[256];

    getcwd(buffer, 256);
    intptr_t address = (intptr_t) mmap(NULL,
                    4,
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS,
                    -1,
                    0);

    printf("%d\n", address);
    return 0;
}

