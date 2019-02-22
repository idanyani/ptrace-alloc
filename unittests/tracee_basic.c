//
// Created by mac on 1/26/19.
//

// Tracee program for tracee_lib_test
#include <unistd.h>
#include <stdio.h>

int main(){
    pid_t child;
    char* args[] = {"date", NULL};

    execv("/bin/date", args);

    return 0;
}

