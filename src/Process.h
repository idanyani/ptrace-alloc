#ifndef PTRACE_ALLOC_PROCESS_H
#define PTRACE_ALLOC_PROCESS_H


#include <sys/types.h>

class TracedProcess {
  public:
    explicit
    TracedProcess(pid_t pid) : pid_(pid), in_kernel_(false), process_started(false) {}

    bool operator<(const TracedProcess& rhs) const {
        return pid_ < rhs.pid_;
    }

    pid_t pid() const {
        return pid_;
    }

    bool isInsideKernel() const {
        return in_kernel_;
    }

    void toggleKernelUser() const {
        in_kernel_ = !in_kernel_;
    }

    bool isStarted() const {
        return process_started;
    }

    void setStarted(bool started) {
        process_started = started;
    }

  private:
    pid_t pid_;
    mutable bool in_kernel_;
    bool process_started;
};

#endif //PTRACE_ALLOC_PROCESS_H
