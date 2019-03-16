#ifndef PTRACE_ALLOC_PTRACETEST_H
#define PTRACE_ALLOC_PTRACETEST_H

#include "Ptrace.h"
#include <memory>        // unique_ptr
#include <unordered_map>
#include <gmock/gmock.h>
#include <gtest/gtest.h>


// Common syscall objects to be created once
static const Syscall kill_syscall ("kill");
static const Syscall close_syscall("close");
static const Syscall write_syscall("write");

/// a matcher to be able to match onSyscall*() with a specific syscall
template <typename SyscallAction>
class SyscallEqMatcher : public ::testing::MatcherInterface<SyscallAction&> {
  public:
    explicit
    SyscallEqMatcher(Syscall syscall) : syscall_(syscall) {}

    void DescribeTo(std::ostream* os) const override {
        *os << "syscall is: " << syscall_;
    }

    void DescribeNegationTo(std::ostream* os) const override {
        *os << "syscall isn't: " << syscall_;
    }

    bool MatchAndExplain(SyscallAction& action,
                         ::testing::MatchResultListener*) const override {
        return action.getSyscall() == syscall_;
    }

  private:
    const Syscall syscall_;
};

template <typename SyscallAction>
inline ::testing::Matcher<SyscallAction&> SyscallEq(const Syscall& syscall) {
    return MakeMatcher(new SyscallEqMatcher<SyscallAction>(syscall));
}


class MockEventCallbacks : public Ptrace::EventCallbacks {
  public:
    MockEventCallbacks();
    ~MockEventCallbacks() override ;

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

    int onSyscallEnter(pid_t pid, Ptrace::SyscallEnterAction& action) override;

    int onSyscallExit(pid_t pid, Ptrace::SyscallExitAction& action) override;

  private:
    bool test_started;  // Used to ignore all the syscalls before the first "kill" in the tracee.
    // Make the tests much easier to use and easy to express expectations,
    //  for example, at the beginning there is a syscall-enter->exit loop of
    //  unknown iterations, and it is hard (impossible?) to express it properly
    //  and keeping it encapsulated.
    // Another way to solve this issue instead of this hack is to reset
    //  expectations on the first "kill" with "Mock::VerifyAndClearExpectations()".
    std::unordered_map<pid_t, bool> is_inside_kernel;
};

class PtraceTest : public testing::Test {
  protected:
    PtraceTest();
    ~PtraceTest() override;

    void sendCommand(char command) {
        ASSERT_EQ(write(command_pipe[1], &command, sizeof(command)), (ssize_t)sizeof(command));
    }

    template<typename Data>
    void readData(Data& data) {
        ASSERT_EQ(read(data_pipe[0], &data, sizeof(data)), (ssize_t) sizeof(data));
    }

    std::unique_ptr<Ptrace> p_ptrace;
    MockEventCallbacks      mock_event_callbacks;

  private:
    void openPipes();
    void closeUnusedPipeEnds();

    void start_tracee(const char* tracee_path);

    int command_pipe[2];
    int data_pipe   [2];
};

#endif //PTRACE_ALLOC_PTRACETEST_H