//
// Created by mac on 2/22/19.
//

#include "Ptrace.h"
#include "PtraceTest.h"

#ifndef PTRACE_ALLOC_TRACEE_LIB_FIFO_TEST_H
#define PTRACE_ALLOC_TRACEE_LIB_FIFO_TEST_H


class SendMessageCallback : public MockEventCallbacks {

  public:
    SendMessageCallback(std::string sys_call_name) : message_sys_call_name(sys_call_name) {}

//    virtual void onStart    (pid_t)                 override;

//    virtual void onExit     (pid_t, int retval)     override;
//    virtual void onTerminate(pid_t, int signal_num) override;
//    virtual void onSignal   (pid_t, int signal_num) override;
//
//    virtual int onSyscallEnter(pid_t, Ptrace::SyscallEnterAction&) override;
    virtual int onSyscallExit (pid_t, Ptrace::SyscallExitAction&)  override;

    // Events callbacks
//    virtual void onFork(pid_t)      override;
//    virtual void onVFork(pid_t)     override;
//    virtual void onVForkDone(pid_t) override;
//    virtual void onClone(pid_t)     override;
//    virtual void onExec(pid_t)      override;

  private:
    std::string message_sys_call_name;
};



#endif //PTRACE_ALLOC_TRACEE_LIB_FIFO_TEST_H
