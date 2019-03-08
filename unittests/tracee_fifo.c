//
// Created by mac on 2/22/19.
//
#include <unistd.h>     // getcwd
#include <sys/mman.h>   // mmap
#include <stdio.h>
#include <memory.h>     // strcmp

int main(int argc, char **argv){

    char buffer[256];
    char* args[] = {"tracee_basic", 0};

    getcwd(buffer, 256);
    intptr_t address = (intptr_t) mmap(NULL,
                    4,
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS,
                    -1,
                    0);

    printf("%d\n", address);
    if(argc > 1 && !strcmp(argv[1], "1")) {
        printf("BEFORE EXEC\n");
        execv("./tracee_basic", args);
    }

    return 0;
}

