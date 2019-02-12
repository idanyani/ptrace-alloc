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

class TraceeLibEventCallbacks : public Ptrace::EventCallbacks {
  public:
    //~TraceeLibEventCallbacks() override;

    void onStart    (pid_t pid)                 override;
    void onExit     (pid_t pid, int retval)     override;
    void onTerminate(pid_t, int signal_num)     override;
    void onSignal   (pid_t pid, int signal_num) override;

    void onSyscallEnter(pid_t pid, Ptrace::SyscallEnterAction& action)  override;
    void onSyscallExit (pid_t pid, Ptrace::SyscallExitAction& action)   override;
};

class TraceeLibTest : public testing::Test {
  protected:
    TraceeLibTest();

    std::unique_ptr<Ptrace> p_ptrace;
//    TraceeLibEventCallbacks tracee_lib_event_callbacks;
    MockEventCallbacks      mock_event_callbacks;
};


#endif //PTRACE_ALLOC_TRACEE_LIB_TEST_H
