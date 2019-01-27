#ifndef PTRACE_ALLOC_PROCESS_H
#define PTRACE_ALLOC_PROCESS_H


#include <sys/types.h>

class TracedProcess {
  public:
    explicit
    TracedProcess(pid_t pid) : pid_(pid), in_kernel_(false), user_signal_handlers_set_(false) {}

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

    bool userSignalHandlersAreSet() const {
        return user_signal_handlers_set_;
    }

    void setuserSignalHandlersSet(bool started) {
        user_signal_handlers_set_ = started;
    }

  private:
    pid_t pid_;
    bool in_kernel_;
    bool user_signal_handlers_set_;
};

#endif //PTRACE_ALLOC_PROCESS_H
