#ifndef PTRACE_ALLOC_PTRACE_H
#define PTRACE_ALLOC_PTRACE_H

#include <string>
#include <stdexcept>

#include "syscall.h"

class Ptrace {
  public:
    class NoChildren : public std::logic_error {
      public:
        NoChildren() : std::logic_error("No more descendants to wait for!") {}
    };

    enum class SyscallDirection {
        ENTRY, EXIT
    };

    Ptrace(const std::string& executable, char* args[]);

    ~Ptrace();

    // non copyable
    Ptrace(const Ptrace&) = delete;

    Ptrace& operator=(const Ptrace&) = delete;

    std::pair<Syscall, SyscallDirection> runUntilSyscallGate();

    std::pair<int, bool> runUntilExit();

    pid_t getChildPid() const {
        return tracee_pid_;
    }

  private:

    enum class TraceeStatus {
        EXITED,
        TERMINATED,
        SIGNALED,
        SYSCALLED,
        CONTINUED
    };

    pid_t waitForDescendant(TraceeStatus& tracee_status, int* entry = nullptr);

    // private data
    pid_t tracee_pid_;
    bool  in_kernel_;
};


#endif //PTRACE_ALLOC_PTRACE_H
