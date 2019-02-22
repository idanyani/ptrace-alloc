#ifndef PTRACE_ALLOC_PTRACE_H
#define PTRACE_ALLOC_PTRACE_H

#include <string>
#include <stdexcept>
#include <sys/types.h>
#include <set>
//#include <bits/unordered_map.h>
#include <unordered_map>

#include "Syscall.h"
#include "Logger.h"
#include "Process.h"

template<typename T>
T handleSyscallReturnValue(T syscall_return_value, unsigned code_line) {
    if (syscall_return_value < 0) {
        throw std::system_error(errno,
                                std::system_category(),
                                std::string("Failed on line:") + std::to_string(code_line));
    }
    return syscall_return_value;
}

#define SAFE_SYSCALL(syscall) \
    handleSyscallReturnValue(syscall, __LINE__)

#define SAFE_SYSCALL_BY_ERRNO(syscall)                      \
    ({                                                      \
        errno = 0;                                          \
        const auto ret = syscall;                           \
        handleSyscallReturnValue(-errno, __LINE__);         \
        ret;                                                \
    }) // https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html

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
        virtual void onSyscallExit (pid_t, SyscallExitAction&)  {}

        // Events callbacks
        virtual void onFork(pid_t)      {}
        virtual void onVFork(pid_t)     {}
        virtual void onVForkDone(pid_t) {}
        virtual void onClone(pid_t)     {}
        virtual void onExec(pid_t)    {}

    };

    Ptrace(const std::string& executable, char* args[], EventCallbacks&);

    ~Ptrace();

    void startTracing();

    // non copyable
    Ptrace(const Ptrace&) = delete;
    Ptrace& operator=(const Ptrace&) = delete;
    void setLoggerVerbosity(Logger::Verbosity verbosity);


  private:

    using ProcessList = std::unordered_map<pid_t, TracedProcess>;
    using ProcessItr = std::unordered_map<pid_t, TracedProcess>::iterator;

    EventCallbacks& event_callbacks_;
    ProcessList     process_list_;
    Logger          logger_;

    void    setSyscall      (const TracedProcess&, Syscall);
    Syscall getSyscall      (const TracedProcess&);
    void    setReturnValue  (const TracedProcess&, long);

    void handleExitedProcess(int status, ProcessItr waited_process);
    void handleTerminatedProcess(int status, ProcessItr waited_process);
    void handleSyscalledProcess(int status, int signal_num, ProcessItr waited_process);
    void handleSignaledProcess(int status, int signal_num, ProcessItr waited_process);
    void handleTrappedProcess(int status, int signal_num, ProcessItr waited_process);
    void handleNewBornProcess(pid_t waited_pid);


    bool isSyscallStop(int sig_num);
    bool startingReturnFromSignal(const TracedProcess& process);
    bool finishingReturnFromSignal(const TracedProcess& process);

};


#endif //PTRACE_ALLOC_PTRACE_H
