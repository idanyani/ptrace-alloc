#include "Ptrace.h"
#include "PtraceTest.h"

#include <sys/types.h>

using ::testing::_;
using ::testing::AnyOf;
using ::testing::Contains;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Not;


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

