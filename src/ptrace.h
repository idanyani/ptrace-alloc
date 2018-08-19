#ifndef PTRACE_ALLOC_PTRACE_H
#define PTRACE_ALLOC_PTRACE_H

#include <string>
#include <stdexcept>
#include <sys/types.h>

#include "syscall.h"
#include "logger.h"


class Ptrace {
  public:
    enum class SyscallDirection {
        ENTRY, EXIT
    };

    /// Base class. derive and implement the event functions you are interested in.
    /// If for some reason "virtual" making a performance impact, we can "easily"
    /// swap this with static polymorphism (templates).
    class EventHandler {
      public:
        virtual ~EventHandler() = default;

        virtual void onExit     (pid_t, int retval)     {}
        virtual void onTerminate(pid_t, int signal_num) {}
        virtual void onSignal   (pid_t, int signal_num) {}

        virtual void onSyscall  (pid_t, const Syscall&, SyscallDirection) {}
    };

    Ptrace(const std::string& executable, char* args[], EventHandler&);

    ~Ptrace();

    void trace();

    // non copyable
    Ptrace(const Ptrace&) = delete;
    Ptrace& operator=(const Ptrace&) = delete;

    void pokeSyscall(const Syscall& syscallToRun);

    pid_t getChildPid() const {
        return tracee_pid_;
    }

  private:
    EventHandler&   eventHandler_;
    pid_t           tracee_pid_;
    bool            in_kernel_;
    Logger          logger_;
};


#endif //PTRACE_ALLOC_PTRACE_H
