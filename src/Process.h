#ifndef PTRACE_ALLOC_PROCESS_H
#define PTRACE_ALLOC_PROCESS_H


#include <sys/types.h>

class TracedProcess {
  public:
    explicit
    TracedProcess(pid_t pid) : pid_(pid), in_kernel_(false), returning_from_signal_(false),
                               has_user_uignal_handlers_(false) {}

    bool operator<(const TracedProcess& rhs) const {
        return pid_ < rhs.pid_;
    }

    pid_t pid() const {
        return pid_;
    }

    bool isInsideKernel() const {
        return in_kernel_;
    }

    void toggleKernelUser() {
        in_kernel_ = !in_kernel_;
    }

    bool returningFromSignal() const {
        return returning_from_signal_;
    }

    void setReturningFromSignal(bool is_returning_from_signal) {
        returning_from_signal_ = is_returning_from_signal;
    }

    bool hasUserSignalHandlers() const {
        return has_user_uignal_handlers_;
    }

    void setHasUserSignalHandlers(bool has_user_signal_handlers) {
        has_user_uignal_handlers_ = has_user_signal_handlers;
    }

  private:
    pid_t pid_;
    bool in_kernel_;
    bool returning_from_signal_;
    bool has_user_uignal_handlers_;
};

#endif //PTRACE_ALLOC_PROCESS_H
