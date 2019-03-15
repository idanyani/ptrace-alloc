//
// Created by mac on 1/26/19.
//

// Tracee program for tracee_lib_test
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>


int main(){
    pid_t child;
    char* args[] = {"date", NULL};

    intptr_t address = (intptr_t) mmap(NULL,
                                       4,
                                       PROT_READ | PROT_WRITE,
                                       MAP_PRIVATE | MAP_ANONYMOUS,
                                       -1,
                                       0);
    printf("%d\n", address);
    execv("/bin/date", args);

    return 0;
}

