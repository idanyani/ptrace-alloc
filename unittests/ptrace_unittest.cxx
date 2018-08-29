#include "Ptrace.h"

#include <wait.h>
#include <cassert>
#include <memory>
#include <unordered_map>
#include <gtest/gtest.h>
#include <gmock/gmock.h>


using ::testing::_;
using ::testing::AnyOf;
using ::testing::AtLeast;
using ::testing::Contains;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Not;

using ::testing::Matcher;
using ::testing::MatcherInterface;
using ::testing::MatchResultListener;

// Common syscall objects to be created once
static const Syscall kill_syscall ("kill");
static const Syscall close_syscall("close");
static const Syscall write_syscall("write");

/// a matcher to be able to match onSyscall*() with a specific syscall
template <typename SyscallAction>
class SyscallEqMatcher : public MatcherInterface<SyscallAction&> {
  public:
    explicit
    SyscallEqMatcher(Syscall syscall) : syscall_(syscall) {}

    void DescribeTo(::std::ostream* os) const override {
        *os << "syscall is: " << syscall_;
    }

    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "syscall isn't: " << syscall_;
    }

    bool MatchAndExplain(SyscallAction& action,
                         MatchResultListener*) const override {
        return action.getSyscall() == syscall_;
    }

  private:
    const Syscall syscall_;
};

template <typename SyscallAction>
inline Matcher<SyscallAction&> SyscallEq(const Syscall& syscall) {
    return MakeMatcher(new SyscallEqMatcher<SyscallAction>(syscall));
}

class MockEventCallbacks : public Ptrace::EventCallbacks {
  public:
    MockEventCallbacks() : test_started(false) {}

    MOCK_METHOD1(onStart        , void(pid_t));
    MOCK_METHOD2(onExit         , void(pid_t, int retval));
    MOCK_METHOD2(onSyscallEnterT, void(pid_t, Ptrace::SyscallEnterAction&));
    MOCK_METHOD2(onSyscallExitT , void(pid_t, Ptrace::SyscallExitAction&));
    MOCK_METHOD2(onTerminate    , void(pid_t, int signal_num));
    MOCK_METHOD2(onSignal       , void(pid_t, int signal_num));

    void onSyscallEnter(pid_t pid, Ptrace::SyscallEnterAction& action) override {
        auto& in_syscall = is_inside_kernel[pid];
        ASSERT_FALSE(in_syscall);
        in_syscall = true;

        if (test_started) {
            onSyscallEnterT(pid, action);
        }
    }

    void onSyscallExit(pid_t pid, Ptrace::SyscallExitAction& action) override {
        auto& in_syscall = is_inside_kernel[pid];
        ASSERT_TRUE(in_syscall);
        in_syscall = false;

        if (test_started) {
            onSyscallExitT(pid, action);
        }
        if (action.getSyscall() == kill_syscall) {
            test_started = true;
        }
    }

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

class PtraceTest : public ::testing::Test {
  protected:
    PtraceTest() : command_pipe{-1, -1}, data_pipe{-1, -1} {
        start_tracee("./tracee");
    }

    ~PtraceTest() override {
        closeFd(command_pipe[0]);
        closeFd(command_pipe[1]);
        closeFd(data_pipe[0]);
        closeFd(data_pipe[1]);
    }

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
    static void closeFd(int& fd) {
        if (fd >= 0)
            close(fd);
        fd = -1;
    }

    void openPipes() {
        ASSERT_EQ(0, pipe(command_pipe));
        ASSERT_EQ(0, pipe(data_pipe));
    }

    void closeUnusedPipeEnds() {
        closeFd(command_pipe[0]);
        closeFd(data_pipe[1]);
    }

    void start_tracee(const char* tracee_path) {
        openPipes();

        char* args[] = {const_cast<char*>(tracee_path), nullptr};
        p_ptrace.reset(new Ptrace(args[0], args, mock_event_callbacks));

        closeUnusedPipeEnds();
    }

    int command_pipe[2];
    int data_pipe   [2];
};


TEST_F(PtraceTest, Syscall) {
    InSequence in_sequence;
    sendCommand(0);

    EXPECT_CALL(mock_event_callbacks,
                onStart);

    EXPECT_CALL(mock_event_callbacks,
                onSyscallEnterT(_, SyscallEq<Ptrace::SyscallEnterAction>(close_syscall)));
    EXPECT_CALL(mock_event_callbacks,
                onSyscallExitT(_, SyscallEq<Ptrace::SyscallExitAction>(close_syscall)));

    EXPECT_CALL(mock_event_callbacks,
                onSyscallEnterT(_, SyscallEq<Ptrace::SyscallEnterAction>(Syscall("exit_group"))));

    EXPECT_CALL(mock_event_callbacks, onExit(_, 0));

    p_ptrace->startTracing();
}

TEST_F(PtraceTest, MmapHijack) {
    InSequence in_sequence;
    sendCommand(1);

    EXPECT_CALL(mock_event_callbacks,
                onStart);

    pid_t tracee_pid;
    EXPECT_CALL(mock_event_callbacks,
                onSyscallEnterT(_, SyscallEq<Ptrace::SyscallEnterAction>(Syscall("mmap"))))
            .WillOnce(Invoke([&](pid_t pid, Ptrace::SyscallEnterAction& action) {
                action.setSyscall(Syscall("getpid"));
                tracee_pid = pid;
            }));
    EXPECT_CALL(mock_event_callbacks,
                onSyscallExitT(_, SyscallEq<Ptrace::SyscallExitAction>(Syscall("getpid"))));

    EXPECT_CALL(mock_event_callbacks,
                onSyscallEnterT(_, SyscallEq<Ptrace::SyscallEnterAction>(write_syscall)));
    EXPECT_CALL(mock_event_callbacks,
                onSyscallExitT(_, SyscallEq<Ptrace::SyscallExitAction>(write_syscall)));

    EXPECT_CALL(mock_event_callbacks,
                onSyscallEnterT(_, SyscallEq<Ptrace::SyscallEnterAction>(Syscall("exit_group"))));

    EXPECT_CALL(mock_event_callbacks, onExit(_, 0));

    p_ptrace->startTracing();

    pid_t received;
    readData(received);
    ASSERT_EQ(tracee_pid, received);
}

TEST_F(PtraceTest, ReturnValueInjection) {
    InSequence in_sequence;
    sendCommand(0);

    const auto injected_value = 42;

    EXPECT_CALL(mock_event_callbacks,
                onStart);

    EXPECT_CALL(mock_event_callbacks,
                onSyscallEnterT(_, SyscallEq<Ptrace::SyscallEnterAction>(close_syscall)));
    EXPECT_CALL(mock_event_callbacks,
                onSyscallExitT(_, SyscallEq<Ptrace::SyscallExitAction>(close_syscall)))
                .WillOnce(Invoke([](pid_t pid, Ptrace::SyscallExitAction& action) {
                    action.setReturnValue(injected_value);
                }));

    EXPECT_CALL(mock_event_callbacks,
                onSyscallEnterT(_, SyscallEq<Ptrace::SyscallEnterAction>(Syscall("exit_group"))));

    EXPECT_CALL(mock_event_callbacks, onExit(_, injected_value));

    p_ptrace->startTracing();
}

TEST_F(PtraceTest, Terminate) {
    InSequence in_sequence;
    sendCommand(2);

    EXPECT_CALL(mock_event_callbacks,
                onStart);

    EXPECT_CALL(mock_event_callbacks,
                onSyscallEnterT(_, SyscallEq<Ptrace::SyscallEnterAction>(Syscall("kill"))));
    EXPECT_CALL(mock_event_callbacks,
                onSyscallExitT(_, SyscallEq<Ptrace::SyscallExitAction>(Syscall("kill"))));

    EXPECT_CALL(mock_event_callbacks, onSignal(_, SIGTERM));
    EXPECT_CALL(mock_event_callbacks, onTerminate);

    p_ptrace->startTracing();
}

TEST_F(PtraceTest, Fork) {
    sendCommand(3);

    std::vector<pid_t> children;
    ON_CALL(mock_event_callbacks,
            onStart)
            .WillByDefault(Invoke([&](pid_t pid) { children.push_back(pid); }));

    EXPECT_CALL(mock_event_callbacks,
                onStart)
                .Times(3);

    EXPECT_CALL(mock_event_callbacks,
                onSyscallEnterT(_,
                                AnyOf(SyscallEq<Ptrace::SyscallEnterAction>(Syscall("fork")),
                                      SyscallEq<Ptrace::SyscallEnterAction>(Syscall("clone")))))
                .Times(2);
    EXPECT_CALL(mock_event_callbacks,
                onSyscallExitT(_,
                               AnyOf(SyscallEq<Ptrace::SyscallExitAction>(Syscall("fork")),
                                     SyscallEq<Ptrace::SyscallExitAction>(Syscall("clone")))))
                .Times(2);

    EXPECT_CALL(mock_event_callbacks,
                onSyscallEnterT(_, SyscallEq<Ptrace::SyscallEnterAction>(write_syscall)))
                .Times(2);
    EXPECT_CALL(mock_event_callbacks,
                onSyscallExitT(_, SyscallEq<Ptrace::SyscallExitAction>(write_syscall)))
                .Times(2);

    EXPECT_CALL(mock_event_callbacks,
                onSyscallEnterT(_, SyscallEq<Ptrace::SyscallEnterAction>(Syscall("exit_group"))))
                .Times(3);

    // these calls might not be called on some platforms (kernel/libc/pthread)
    EXPECT_CALL(mock_event_callbacks,
                onSyscallEnterT(_, SyscallEq<Ptrace::SyscallEnterAction>(Syscall("set_robust_list"))))
                .Times(2);
    EXPECT_CALL(mock_event_callbacks,
                onSyscallExitT(_, SyscallEq<Ptrace::SyscallExitAction>(Syscall("set_robust_list"))))
                .Times(2);

    EXPECT_CALL(mock_event_callbacks,
                onSyscallEnterT(_, SyscallEq<Ptrace::SyscallEnterAction>(close_syscall)));
    EXPECT_CALL(mock_event_callbacks,
                onSyscallExitT(_, SyscallEq<Ptrace::SyscallExitAction>(close_syscall)));

    EXPECT_CALL(mock_event_callbacks, onExit(_, 0))
                .Times(3);

    p_ptrace->startTracing();

    EXPECT_EQ(3, children.size());

    std::array<pid_t, 2> sent_pids;
    for (auto& pid : sent_pids) {
        readData(pid);
        EXPECT_THAT(children, Contains(pid));
    }

    EXPECT_THAT(sent_pids, Not(Contains(children.front()))); // no one sent the first child's pid

}
