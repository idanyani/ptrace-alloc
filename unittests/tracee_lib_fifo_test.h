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

    virtual int onSyscallExit (pid_t, Ptrace::SyscallExitAction&)  override;

  private:
    std::string message_sys_call_name;
};

class SendSignalOnMmapCallback : public MockEventCallbacks {
  public:
    virtual int onSyscallExit (pid_t, Ptrace::SyscallExitAction&)  override;
};


#endif //PTRACE_ALLOC_TRACEE_LIB_FIFO_TEST_H
