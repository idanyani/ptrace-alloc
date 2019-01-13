//
// Created by mac on 11/21/18.
//
#include <unistd.h>
#include <stdio.h>
#include <signal.h> // sigaction, struct sigaction
#include <sys/types.h> // mkfifo
#include <sys/stat.h> // mkfifo
#include <string.h> // strcpy, strcat
#include <errno.h>
//#include "tracee_lib.h"


void setUserSignals();
void allocate_handler(int address);
void create_fifo(int address);

void setUserSignals(){
    struct sigaction allocate_action, create_fifo_action;
    allocate_action.sa_handler = allocate_handler;
    sigemptyset (&allocate_action.sa_mask);
    allocate_action.sa_flags = 0;

    create_fifo_action.sa_handler = create_fifo;
    sigemptyset (&create_fifo_action.sa_mask);
    create_fifo_action.sa_flags = 0;

    sigaction (SIGUSR1, &allocate_action, NULL);
    sigaction (SIGUSR2, &create_fifo_action, NULL);
}

// TODO: replace hard-coded path with generic one
// TODO: research tmpfile()/tempnnam() for creating temporal place for fifo
int make_fifo_for_process(){
    // /home/mac/CLionProjects/ptrace-alloc/fifo/
    int res;
    char fifo_path[80];
    char pid_str[20];
    sprintf(pid_str, "%d", getpid());

    strcpy(fifo_path, "/tmp/fifo/");
    strcat(fifo_path, pid_str);

    // TODO: check permissions at
    res = mkfifo(fifo_path, S_IRUSR | S_IWOTH);
    if(res < 0)
        printf("make_fifo_for_process failed: %s\n", strerror(errno));
    return res;
}

/*
 * called upon execve before main()
 * sets signal handlers for the child (signal handlers are reset upon execv)
 * creates fifo identified with new process's id if wasn't created already
*/
__attribute__((constructor)) void tracee_begin(){

    printf("tracee_begin: %d\n", getpid());
    setUserSignals();
    kill(getpid(), 0);

}

// TODO: add destructor to close FIFO and remove it

/*
 * siguser1_handler
 * reads from fifo data sent by tracer
 * allocates memory
 * */

void allocate_handler(int address){
    // TODO: implement
    printf("allocate_handler: %d\n", getpid());
}

/*
 * siguser2_handler
 * allocates fifo
 * needed for case when new process doesn't call to execve
 * if that case, the handler is inherited from it's parent
 * */

void create_fifo(int address){
    int fifo_create_res = make_fifo_for_process();
    printf("create_fifo: mmake_fifo_for_process res  %d\n", fifo_create_res);
}

//

