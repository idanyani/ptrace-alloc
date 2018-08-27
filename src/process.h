#ifndef PTRACE_ALLOC_PROCESS_H
#define PTRACE_ALLOC_PROCESS_H


#include <sys/types.h>

class TracedProcess {
  public:
    explicit
    TracedProcess(pid_t pid) : pid_(pid), flags{.in_syscall_=false,
                                                .initialized_=false} {}

    bool operator<(const TracedProcess& rhs) const {
        return pid_ < rhs.pid_;
    }

    // TODO: private:
    pid_t   pid_;

    mutable struct {//TODO: bitmap?
        bool in_syscall_;
        bool initialized_;
    } flags;
};

#endif //PTRACE_ALLOC_PROCESS_H
