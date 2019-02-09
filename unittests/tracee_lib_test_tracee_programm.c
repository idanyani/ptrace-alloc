//
// Created by mac on 1/26/19.
//

// Tracee program for tracee_lib_test
#include <unistd.h>
#include <stdio.h>

void childFunction(){

}

int main(){

    char* args[] = {"date"};
    printf("%d\n", getpid());
    execv("/bin/date", args);
}

