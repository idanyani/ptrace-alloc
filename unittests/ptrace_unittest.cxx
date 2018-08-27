#include "ptrace.h"

#include <wait.h>
#include <cassert>
#include <memory>
#include <gtest/gtest.h>
#include <gmock/gmock.h>


using ::testing::_;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::InvokeWithoutArgs;

enum class SyscallDirection {
    ENTRY, EXIT
};

class MockEventCallbacks : public Ptrace::EventCallbacks {
  public:
    MockEventCallbacks() : inside_kernel(false) {}

    MOCK_METHOD2(onExit        , void(pid_t, int retval));
    MOCK_METHOD3(onSyscall     , void(pid_t, const Syscall&, SyscallDirection direction));

    void onSyscallEnter(pid_t pid, Syscall syscall) override {
        ASSERT_FALSE(inside_kernel);
        inside_kernel = true;
        onSyscall(pid, syscall, SyscallDirection::ENTRY);
    }

    void onSyscallExit(pid_t pid, Syscall syscall) override {
        ASSERT_TRUE(inside_kernel);
        inside_kernel = false;
        onSyscall(pid, syscall, SyscallDirection::EXIT);
    }

  private:
    bool inside_kernel;
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
        const Syscall read_syscall("read");

        EXPECT_CALL(mock_event_callbacks,
                    onSyscall(_,
                              read_syscall,
                              SyscallDirection::ENTRY));
        EXPECT_CALL(mock_event_callbacks,
                    onSyscall(_,
                              read_syscall,
                              SyscallDirection::EXIT));

        ASSERT_EQ(write(command_pipe[1], &command, sizeof(command)), (ssize_t)sizeof(command));
    }

    void receivePid() {
        const Syscall write_syscall("write");

        EXPECT_CALL(mock_event_callbacks,
                    onSyscall(_,
                              write_syscall,
                              SyscallDirection::ENTRY));
        EXPECT_CALL(mock_event_callbacks,
                    onSyscall(_, write_syscall, SyscallDirection::EXIT))
                .WillOnce(Invoke([=](pid_t pid, Syscall, SyscallDirection) {
                    pid_t received;
                    ASSERT_EQ(read(data_pipe[0], &received, sizeof(received)),
                              (ssize_t) sizeof(received));
                    ASSERT_EQ(pid, received);
                }));
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
        char* args[] = {const_cast<char*>(tracee_path), nullptr};

        EXPECT_CALL(mock_event_callbacks,
                    onSyscall(_, _, _)).Times(AtLeast(10));

        const Syscall kill_syscall("kill");

        EXPECT_CALL(mock_event_callbacks,
                    onSyscall(_,
                              kill_syscall,
                              SyscallDirection::ENTRY));
        EXPECT_CALL(mock_event_callbacks,
                    onSyscall(_,
                              kill_syscall,
                              SyscallDirection::EXIT));

        const Syscall close_syscall("close");
        for (auto i = 0; i < 2; ++i) {
            EXPECT_CALL(mock_event_callbacks,
                        onSyscall(_,
                                  close_syscall,
                                  SyscallDirection::ENTRY));
            EXPECT_CALL(mock_event_callbacks,
                        onSyscall(_,
                                  close_syscall,
                                  SyscallDirection::EXIT));
        }

        openPipes();

        p_ptrace.reset(new Ptrace(args[0], args, mock_event_callbacks));

        closeUnusedPipeEnds();
    }

    InSequence in_sequence; // for making the constraints ordered

    int command_pipe[2];
    int data_pipe   [2];
};


TEST_F(PtraceTest, BasicTest) {
    sendCommand(0);

    const Syscall close_syscall("close");

    EXPECT_CALL(mock_event_callbacks,
                onSyscall(_,
                          close_syscall,
                          SyscallDirection::ENTRY));
    EXPECT_CALL(mock_event_callbacks,
                onSyscall(_,
                          close_syscall,
                          SyscallDirection::EXIT));

    EXPECT_CALL(mock_event_callbacks,
                onSyscall(_,
                          Syscall("exit_group"),
                          SyscallDirection::ENTRY));

    EXPECT_CALL(mock_event_callbacks, onExit(_, 0));

    p_ptrace->startTracing();
}

TEST_F(PtraceTest, MmapHijackTest) {
    sendCommand(1);

    EXPECT_CALL(mock_event_callbacks,
                onSyscall(_,
                          Syscall("mmap"),
                          SyscallDirection::ENTRY))
            .WillOnce(Invoke([=](pid_t pid, Syscall, SyscallDirection) {
                p_ptrace->pokeSyscall(pid /*TODO: the interface should be clearer*/,
                                      Syscall("getpid"));
            }));
    EXPECT_CALL(mock_event_callbacks,
                onSyscall(_,
                          Syscall("getpid"),
                          SyscallDirection::EXIT));

    receivePid();

    EXPECT_CALL(mock_event_callbacks,
                onSyscall(_,
                          Syscall("exit_group"),
                          SyscallDirection::ENTRY));

    EXPECT_CALL(mock_event_callbacks, onExit(_, 0));

    p_ptrace->startTracing();
}
