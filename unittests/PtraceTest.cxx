#include "PtraceTest.h"
#include <unistd.h>
#include <gmock/gmock.h>


MockEventCallbacks::MockEventCallbacks() : test_started(false) {}

void MockEventCallbacks::onSyscallEnter(pid_t pid, Ptrace::SyscallEnterAction& action) {
    auto& in_syscall = is_inside_kernel[pid];
    ASSERT_FALSE(in_syscall);
    in_syscall = true;

    if (test_started) {
        onSyscallEnterT(pid, action);
    }
}

void MockEventCallbacks::onSyscallExit(pid_t pid, Ptrace::SyscallExitAction& action) {
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

MockEventCallbacks::~MockEventCallbacks() = default;


PtraceTest::PtraceTest() : command_pipe{-1, -1}, data_pipe{-1, -1} {
    start_tracee("./tracee");
}

static
void closeFd(int& fd) {
    if (fd >= 0)
        close(fd);
    fd = -1;
}

PtraceTest::~PtraceTest()  {
    closeFd(command_pipe[0]);
    closeFd(command_pipe[1]);
    closeFd(data_pipe[0]);
    closeFd(data_pipe[1]);
}

void PtraceTest::openPipes() {
    ASSERT_EQ(0, pipe(command_pipe));
    ASSERT_EQ(0, pipe(data_pipe));
}

void PtraceTest::closeUnusedPipeEnds() {
    closeFd(command_pipe[0]);
    closeFd(data_pipe[1]);
}

void PtraceTest::start_tracee(const char* tracee_path) {
    openPipes();

    char* args[] = {const_cast<char*>(tracee_path), nullptr};
    p_ptrace.reset(new Ptrace(args[0], args, mock_event_callbacks));

    closeUnusedPipeEnds();
}
