//
// Created by mac on 2/6/19.
//

#ifndef PTRACE_ALLOC_TRACEE_LIB_TEST_H
#define PTRACE_ALLOC_TRACEE_LIB_TEST_H

#include "Ptrace.h"
#include "PtraceTest.h"
#include <memory>        // unique_ptr
#include <unordered_map>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <tracee_lib.h>

class TraceeLibMockEventCallbacks : public TraceeLibEventCallbacks {
  public:
    TraceeLibMockEventCallbacks() = default ;
    ~TraceeLibMockEventCallbacks() override = default;

    MOCK_METHOD1(onStart        , void(pid_t));
    MOCK_METHOD1(onFork         , void(pid_t));
    MOCK_METHOD1(onClone        , void(pid_t));
    MOCK_METHOD1(onVFork        , void(pid_t));
    MOCK_METHOD1(onExec        , void(pid_t));
    MOCK_METHOD1(onVForkDone        , void(pid_t));
    MOCK_METHOD2(onExit         , void(pid_t, int retval));
    MOCK_METHOD2(onSyscallEnterT, void(pid_t, Ptrace::SyscallEnterAction&));
    MOCK_METHOD2(onSyscallExitT , void(pid_t, Ptrace::SyscallExitAction&));
    MOCK_METHOD2(onTerminate    , void(pid_t, int signal_num));
    MOCK_METHOD2(onSignal       , void(pid_t, int signal_num));

    void onSyscallEnter(pid_t pid, Ptrace::SyscallEnterAction& action) override;

  protected:
    virtual int onSyscallExitInner(pid_t, Ptrace::SyscallExitAction&) override;
};


std::unique_ptr<Ptrace> initPtrace(char** args, TraceeLibMockEventCallbacks& mock_event_callbacks);



#endif //PTRACE_ALLOC_TRACEE_LIB_TEST_H
