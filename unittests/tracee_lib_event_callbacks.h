//
// Created by mac on 1/7/19.
//

#include "Ptrace.h"
#include <unistd.h>

#ifndef PTRACE_ALLOC_TRACEE_LIB_EVENT_CALLBACKS_H
#define PTRACE_ALLOC_TRACEE_LIB_EVENT_CALLBACKS_H


class TraceeLibEventCallbacks : public Ptrace::EventCallbacks {
  public:
    ~TraceeLibEventCallbacks() override;

    void onStart    (pid_t pid);
    void onExit     (pid_t pid, int retval);
    void onTerminate(pid_t, int signal_num) {}
    void onSignal   (pid_t pid, int signal_num);

    void onSyscallEnter(pid_t pid, Ptrace::SyscallEnterAction& action);
    void onSyscallExit (pid_t pid, Ptrace::SyscallExitAction& action);
};


#endif //PTRACE_ALLOC_TRACEE_LIB_EVENT_CALLBACKS_H
