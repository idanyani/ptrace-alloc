//
// Created by mac on 2/16/19.
//
#include <unistd.h>
#include <stdio.h>

int main(){
    pid_t child;
    //char* args[] = {"./tracee_basic", 0}; // FIXME: neeed to putenv for every new born process
    char* args[] = {"/bin/date", 0}; // FIXME: neeed to putenv for every new born process
    char* envp[] = {"LD_PRELOAD=/home/mac/CLionProjects/ptrace-alloc/cmake-build-debug/TraceeLib/libtracee_l.so", 0};

    child = fork();
    if(child == 0)
        execve("./tracee_basic", args, envp);
    else
        printf("%d\n", getpid());

}
