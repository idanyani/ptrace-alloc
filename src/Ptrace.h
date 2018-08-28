#ifndef PTRACE_ALLOC_PTRACE_H
#define PTRACE_ALLOC_PTRACE_H

#include <string>
#include <stdexcept>
#include <sys/types.h>
#include <set>

#include "Syscall.h"
#include "Logger.h"
#include "Process.h"


class Ptrace {
  public:

    /// Base class. derive and implement the event functions you are interested in.
    /// If for some reason "virtual" making a performance impact, we can "easily"
    /// swap this with static polymorphism (templates).
    class EventCallbacks {
      public:
        virtual ~EventCallbacks() = default;

        virtual void onExit     (pid_t, int retval)     {}
        virtual void onTerminate(pid_t, int signal_num) {}
        virtual void onSignal   (pid_t, int signal_num) {}

        virtual void onSyscallEnter(pid_t, Syscall) {}
        virtual void onSyscallExit (pid_t, Syscall) {}

#if 0 //TODO:
    protected:
        Syscall getSyscall();
        void setSyscall(Syscall);
        void setReturnValue();
#endif
    };

    Ptrace(const std::string& executable, char* args[], EventCallbacks&);

    ~Ptrace();

    void startTracing();

    // non copyable
    Ptrace(const Ptrace&) = delete;
    Ptrace& operator=(const Ptrace&) = delete;

    void pokeSyscall(pid_t pid, Syscall syscall_to_run);

  private:
    using ProcessList = std::set<TracedProcess>; //TODO: replace with hash-table?

    EventCallbacks& event_callbacks_;
    ProcessList     process_list_;
    Logger          logger_;
};


#endif //PTRACE_ALLOC_PTRACE_H
