//
// Created by mac on 11/21/18.
//
#include <sstream>
#include <iostream>

#include <unistd.h>
#include <signal.h>     // sigaction, struct sigaction
#include <sys/types.h>  // mkfifo, open
#include <sys/stat.h>   // mkfifo, open
#include <errno.h>      // errno
#include <fcntl.h>      // open
#include <cassert>
#include <cstring>      // strerror
#include <sys/ptrace.h>
#include <sys/user.h>   // struct user
#include <cstddef>      // offsetof

#include "tracee_lib.h"
#include "tracee_lib_defines.h"
#include "tracee_server.h"

TraceeServer tracee_server;

void allocate_handler(int signal_handler_arg);
void create_fifo(int signal_handler_arg);

int fifo_fd;
char fifo_path[80];


int traceeHandleSyscallReturnValue(int syscall_return_value, unsigned int code_line) {
    if (syscall_return_value < 0) {
        throw std::system_error(errno,
                                std::system_category(),
                                std::string(std::to_string(getpid()) + " Failed on line:") + std::to_string(code_line));

    }
    return syscall_return_value;
}

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
 * called upon execve before main()
 * sets signal handlers for the child (signal handlers are reset upon execv)
 * creates fifo identified with new process's id if wasn't created already
*/
__attribute__((constructor)) void tracee_begin(){

    pid_t tracee_pid = getpid();
    printf("Tracee lib constructor called for pid=%d\n", tracee_pid);

    setUserSignals();

    TRACEE_SAFE_SYSCALL(kill(tracee_pid, 0));
}

__attribute__((destructor)) void tracee_end(){

    pid_t tracee_pid = getpid();
    printf("Tracee lib destructor called for pid=%d\n", tracee_pid);

    int fifo_exists = access(fifo_path, F_OK);

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
void allocate_handler(int signal_handler_arg){
    tracee_server.serveRequest(fifo_fd);
}

/*
 * siguser2_handler
 * allocates fifo
 * needed for case when new process doesn't call to execve
 * if that case, the handler is inherited from it's parent
 * */
void create_fifo(int signal_handler_arg){

    char pid_str[20];
    int fifo_exists;
    pid_t pid = getpid();

    sprintf(pid_str, "%d", pid);

    strcpy(fifo_path, "/tmp/ptrace_fifo/");
    strcat(fifo_path, pid_str);

    fifo_exists = access(fifo_path, F_OK);

    if(fifo_exists < 0 && errno == ENOENT) {            // if FIFO has not been created yet for the process, create it
        TRACEE_SAFE_SYSCALL(mkfifo(fifo_path, 0666));
        printf("FIFO for pid=%d has not been created yet, creating a new FIFO at path %s\n",
               pid,
               fifo_path);
    }
    if( fifo_fd == 0 ){                                 // assume that we never open FIFO at fd = 0
        //int close_res = close(3);                     // Idan said it's okay not to close FIFO when execv

        fifo_fd = TRACEE_SAFE_SYSCALL(open(fifo_path, O_RDWR | O_NONBLOCK));
        printf("FIFO is now opened for pid=%d, file descriptors's index is %d\n",
               pid,
               fifo_fd);
    }
}
