#ifndef PTRACE_ALLOC_PROCESS_H
#define PTRACE_ALLOC_PROCESS_H


#include <sys/types.h>

class TracedProcess {
  public:
    explicit
    TracedProcess(pid_t pid) : pid_(pid), in_syscall_(false) {}

    bool operator<(const TracedProcess& rhs) const {
        return pid_ < rhs.pid_;
    }

    // TODO: private:
    pid_t pid_;
    mutable bool in_syscall_;
};

#endif //PTRACE_ALLOC_PROCESS_H
