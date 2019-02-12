//
// Created by mac on 1/26/19.
//

// Tracee program for tracee_lib_test
#include <unistd.h>
#include <stdio.h>

void childFunction(){

}

int main(){

    pid_t child;
    char* args[] = {"date"};
    execv("/bin/date", args);

    // FIXME comment in
    /*
    child = fork();
    if(child == 0)
        execv("/bin/date", args);
    else
        printf("%d\n", getpid());
        */
}

