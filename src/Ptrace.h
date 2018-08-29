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

    class SyscallAction {
        friend class Ptrace;
      public:
        Syscall getSyscall() const;

      protected:
        SyscallAction(Ptrace& ptrace, const TracedProcess& tracee)
                : ptrace_(ptrace), tracee_(tracee) {}

        Ptrace&              ptrace_;
        const TracedProcess& tracee_;
    };

    class SyscallEnterAction : public SyscallAction {
        friend class Ptrace;
        using SyscallAction::SyscallAction;

      public:
        void setSyscall(Syscall);
    };

    class SyscallExitAction : public SyscallAction {
        friend class Ptrace;
        using SyscallAction::SyscallAction;
      public:
        void setReturnValue(long);
    };

    /// Base class. derive and implement the event functions you are interested in.
    /// If for some reason "virtual" making a performance impact, we can "easily"
    /// swap this with static polymorphism (templates).
    class EventCallbacks {
      public:
        virtual ~EventCallbacks() = default;

        virtual void onStart    (pid_t) {} // TODO: inform if this is a forked or cloned child?
                                           // TODO: inform the parend pid?
        virtual void onExit     (pid_t, int retval)     {}
        virtual void onTerminate(pid_t, int signal_num) {}
        virtual void onSignal   (pid_t, int signal_num) {}

        virtual void onSyscallEnter(pid_t, SyscallEnterAction&) {}
        virtual void onSyscallExit (pid_t, SyscallExitAction&) {}

    };

    Ptrace(const std::string& executable, char* args[], EventCallbacks&);

    ~Ptrace();

    void startTracing();

    // non copyable
    Ptrace(const Ptrace&) = delete;
    Ptrace& operator=(const Ptrace&) = delete;

  private:
    void    setSyscall      (const TracedProcess&, Syscall);
    Syscall getSyscall      (const TracedProcess&);
    void    setReturnValue  (const TracedProcess&, long);

    using ProcessList = std::set<TracedProcess>; //TODO: replace with hash-table?

    EventCallbacks& event_callbacks_;
    ProcessList     process_list_;
    Logger          logger_;
};


#endif //PTRACE_ALLOC_PTRACE_H
