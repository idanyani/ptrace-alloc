//
// Created by mac on 2/16/19.
//
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
    pid_t child;

    char* args[] = {"tracee_basic", 0};

    child = fork();
    if(child == 0) {
        execv("./tracee_basic", args);
    }
    else
        printf("%d\n", getpid());

}
