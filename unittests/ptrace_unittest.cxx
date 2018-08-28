#include "Ptrace.h"

#include <wait.h>
#include <cassert>
#include <memory>
#include <gtest/gtest.h>
#include <gmock/gmock.h>


using ::testing::_;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::Invoke;

enum class SyscallDirection {
    ENTRY, EXIT
};

// Common syscall objects to be created once
static const Syscall kill_syscall ("kill");
static const Syscall close_syscall("close");
static const Syscall write_syscall("write");


class MockEventCallbacks : public Ptrace::EventCallbacks {
  public:
    MockEventCallbacks() : test_started(false), inside_kernel(false) {}

    MOCK_METHOD2(onExit        , void(pid_t, int retval));
    MOCK_METHOD3(onSyscall     , void(pid_t, const Syscall&, SyscallDirection direction));
    MOCK_METHOD2(onTerminate   , void(pid_t, int signal_num));
    MOCK_METHOD2(onSignal      , void(pid_t, int signal_num));

    void onSyscallEnter(pid_t pid, Syscall syscall) override {
        ASSERT_FALSE(inside_kernel);
        inside_kernel = true;

        if (test_started) {
            onSyscall(pid, syscall, SyscallDirection::ENTRY);
        }
    }

    void onSyscallExit(pid_t pid, Syscall syscall) override {
        ASSERT_TRUE(inside_kernel);
        inside_kernel = false;

        if (test_started) {
            onSyscall(pid, syscall, SyscallDirection::EXIT);
        }
        if (syscall == kill_syscall) {
            test_started = true;
        }
    }

  private:
    bool test_started;  // Used to ignore all the syscalls before the first "kill" in the tracee.
                        // Make the tests much easier to use and easy to express expectations
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
                onSyscall(_, close_syscall, SyscallDirection::ENTRY));
    EXPECT_CALL(mock_event_callbacks,
                onSyscall(_, close_syscall, SyscallDirection::EXIT));

    EXPECT_CALL(mock_event_callbacks,
                onSyscall(_, Syscall("exit_group"), SyscallDirection::ENTRY));

    EXPECT_CALL(mock_event_callbacks, onExit(_, 0));

    p_ptrace->startTracing();
}

TEST_F(PtraceTest, MmapHijack) {
    InSequence in_sequence;
    sendCommand(1);

    pid_t tracee_pid;
    EXPECT_CALL(mock_event_callbacks,
                onSyscall(_, Syscall("mmap"), SyscallDirection::ENTRY))
            .WillOnce(Invoke([&](pid_t pid, Syscall, SyscallDirection) {
                p_ptrace->pokeSyscall(pid /*TODO: the interface should be clearer*/,
                                      Syscall("getpid"));
                tracee_pid = pid;
            }));
    EXPECT_CALL(mock_event_callbacks,
                onSyscall(_, Syscall("getpid"), SyscallDirection::EXIT));

    EXPECT_CALL(mock_event_callbacks,
                onSyscall(_, write_syscall, SyscallDirection::ENTRY));
    EXPECT_CALL(mock_event_callbacks,
                onSyscall(_, write_syscall, SyscallDirection::EXIT));

    EXPECT_CALL(mock_event_callbacks,
                onSyscall(_, Syscall("exit_group"), SyscallDirection::ENTRY));

    EXPECT_CALL(mock_event_callbacks, onExit(_, 0));

    p_ptrace->startTracing();

    pid_t received;
    readData(received);
    ASSERT_EQ(tracee_pid, received);
}

TEST_F(PtraceTest, Terminate) {
    InSequence in_sequence;
    sendCommand(2);

    EXPECT_CALL(mock_event_callbacks,
                onSyscall(_, Syscall("kill"), SyscallDirection::ENTRY));
    EXPECT_CALL(mock_event_callbacks,
                onSyscall(_, Syscall("kill"), SyscallDirection::EXIT));

    EXPECT_CALL(mock_event_callbacks, onSignal(_, SIGTERM));
    EXPECT_CALL(mock_event_callbacks, onTerminate);

    p_ptrace->startTracing();
}
