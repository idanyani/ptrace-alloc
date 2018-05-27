#ifndef PTRACE_ALLOC_PTRACE_H
#define PTRACE_ALLOC_PTRACE_H

#include <string>
#include <stdexcept>


using std::string;


class Ptrace {
  public:
    class NoChildren : public std::logic_error {
      public:
        NoChildren() : std::logic_error("No more descendants to wait for!") {}
    };

    enum class TraceeStatus {
        EXITED,
        TERMINATED,
        SIGNALED,
        SYSCALLED,
        CONTINUED
    };

    enum class SyscallDirection {
        ENTRY, EXIT
    };

    Ptrace(const std::string& executable, char* args[]);

    ~Ptrace();

    Ptrace(const Ptrace&) = delete;

    SyscallDirection runUntilSyscallGate();

    pid_t getPid() const {
        return tracee_pid_;
    }

  private:

    pid_t waitForDescendant(TraceeStatus& tracee_status);

    // private data
    pid_t   tracee_pid_;
    bool    in_kernel_;
};


#endif //PTRACE_ALLOC_PTRACE_H
