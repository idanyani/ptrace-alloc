//
// Created by mac on 1/26/19.
//

// Tracee program for tracee_lib_test
#include <unistd.h>
#include <stdio.h>

int main(){
    pid_t child;
    char* args[] = {"date"};
    // FIXME neew to add poking of LD preload upon execv
    char* envp[] = {"LD_PRELOAD=/home/mac/CLionProjects/ptrace-alloc/cmake-build-debug/TraceeLib/libtracee_l.so", 0};
    execve("/bin/date", args, envp);

    return 0;
}

