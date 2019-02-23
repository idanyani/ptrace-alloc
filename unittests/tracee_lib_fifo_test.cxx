//
// Created by mac on 2/22/19.
//

#include <string>       // string
#include <sstream>      // stringstream

#include <sys/types.h>  // open
#include <sys/stat.h>   // open
#include <fcntl.h>      // open
#include <unistd.h>     // write

#include <signal.h>     // signal numbers

#include "tracee_lib_fifo_test.h"

std::string getFifoPathForTracee(pid_t tracee_pid){
    std::stringstream path_buffer;
    path_buffer << "/tmp/ptrace_fifo/" << tracee_pid;

    return path_buffer.str();
}

void TraceeLibFifoTest::onSignal(pid_t pid, int signal_num) {
    EventCallbacks::onSignal(pid , signal_num);
}

int TraceeLibFifoTest::onSyscallExit(pid_t pid, Ptrace::SyscallExitAction& syscall_action) {
    std::string fifo_path = getFifoPathForTracee(pid);
    int fifo_fd = 0;
    std::stringstream message;

    const TracedProcess& tracee = syscall_action.getTracee();
    if(tracee.returningFromSignal()){
        return 0;
    }
    else if(syscall_action.getSyscall().toString() == std::string("getcwd")) {
        try {
            message << "Message for tracee with pid " << pid  << '\0';
            fifo_fd = SAFE_SYSCALL(open(fifo_path.c_str(), O_NONBLOCK | O_WRONLY));
        } catch(const std::system_error& e){
            fifo_fd = 0;
            if(e.code().value() != ENOENT && e.code().value() != ENXIO) // if the tracee hasn't opened the FIFO for reading yet
                throw e;                                                // or hasn't created one yet, ignore
        }

        if(fifo_fd > 0){                                                // if open succeeded, write
            SAFE_SYSCALL(write(fifo_fd, message.str().c_str(), message.str().size()));
            SAFE_SYSCALL(close(fifo_fd));
        }

        return SIGUSR1;                                                 // return SIGUSR1 in order for signal user 1
    }                                                                   // to be injected to tracee

    return 0;
}
