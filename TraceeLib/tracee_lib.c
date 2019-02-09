//
// Created by mac on 11/21/18.
//
#include <unistd.h>
#include <stdio.h>
#include <signal.h> // sigaction, struct sigaction
#include <sys/types.h> // mkfifo, opnen
#include <sys/stat.h> // mkfifo, open
#include <string.h> // strcpy, strcat
#include <errno.h> // errno
#include <fcntl.h> // open

#include "tracee_lib.h"

void allocate_handler(int address);
void create_fifo(int address);

int traceeHandleSyscallReturnValue(int syscall_return_value, unsigned int code_line) {
    if (syscall_return_value < 0) {
        printf("system call failed at line %d with error: %s\n", code_line, strerror(errno));
    }
    return syscall_return_value;
}

#define TRACEE_SAFE_SYSCALL(syscall) \
({int _ret_val = traceeHandleSyscallReturnValue(syscall, __LINE__); _ret_val;})


char fifo_path[80];
int fifo_fd;

void setUserSignals(){
    struct sigaction allocate_action, create_fifo_action;
    allocate_action.sa_handler = allocate_handler;
    sigemptyset (&allocate_action.sa_mask);
    allocate_action.sa_flags = 0;

    create_fifo_action.sa_handler = create_fifo;
    sigemptyset (&create_fifo_action.sa_mask);
    create_fifo_action.sa_flags = 0;

    TRACEE_SAFE_SYSCALL(sigaction (SIGUSR1, &allocate_action, NULL));
    TRACEE_SAFE_SYSCALL(sigaction (SIGUSR2, &create_fifo_action, NULL));
}

/*
void make_fifo_for_process(){
    char pid_str[20];
    sprintf(pid_str, "%d", getpid());

    strcpy(fifo_path, "/tmp/fifo/");
    strcat(fifo_path, pid_str);

    TRACEE_SAFE_SYSCALL(mkfifo(fifo_path, 0666));
    fifo_fd = TRACEE_SAFE_SYSCALL(open(fifo_path, O_RDWR | O_NONBLOCK));
    printf("%d id fifo_id=%d\n", getpid(), fifo_fd);
}
*/
/*
 * called upon execve before main()
 * sets signal handlers for the child (signal handlers are reset upon execv)
 * creates fifo identified with new process's id if wasn't created already
*/
__attribute__((constructor)) void tracee_begin(){
    setUserSignals();
    kill(getpid(), 0);

}

__attribute__((destructor)) void tracee_end(){
    printf("tracee_end: %d fifo_fd: %d\n", getpid(), fifo_fd);
    int fifo_exists = TRACEE_SAFE_SYSCALL(access(fifo_path, F_OK));

    if(fifo_exists == 0) {
        struct stat fifo_stat;
        TRACEE_SAFE_SYSCALL(fstat(fifo_fd, &fifo_stat));
        TRACEE_SAFE_SYSCALL(close(fifo_fd));
        TRACEE_SAFE_SYSCALL(unlink(fifo_path));
    }
}
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
    //make_fifo_for_process();
    char pid_str[20];
    sprintf(pid_str, "%d", getpid());

    strcpy(fifo_path, "/tmp/fifo/");
    strcat(fifo_path, pid_str);

    TRACEE_SAFE_SYSCALL(mkfifo(fifo_path, 0666));
    fifo_fd = TRACEE_SAFE_SYSCALL(open(fifo_path, O_RDWR | O_NONBLOCK));
    printf("%d id fifo_id=%d\n", getpid(), fifo_fd);
}
