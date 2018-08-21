#include "ptrace.h"

#include <wait.h>
#include <cassert>
#include <memory>
#include <gtest/gtest.h>
#include <gmock/gmock.h>


using ::testing::_;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::InvokeWithoutArgs;

class MockEventHandler : public Ptrace::EventHandler {
  public:
    MOCK_METHOD2(onExit     , void(pid_t, int retval));
    MOCK_METHOD3(onSyscall  , void(pid_t, const Syscall&, Ptrace::SyscallDirection));
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

        EXPECT_CALL(mock_event_handler,
                    onSyscall(p_ptrace->getChildPid(),
                              read_syscall,
                              Ptrace::SyscallDirection::ENTRY));
        EXPECT_CALL(mock_event_handler,
                    onSyscall(p_ptrace->getChildPid(),
                              read_syscall,
                              Ptrace::SyscallDirection::EXIT));

        ASSERT_EQ(write(command_pipe[1], &command, sizeof(command)), (ssize_t)sizeof(command));
    }

    template <typename Data>
    void receiveData(Data expected) {
        const Syscall write_syscall("write");

        EXPECT_CALL(mock_event_handler,
                    onSyscall(p_ptrace->getChildPid(),
                              write_syscall,
                              Ptrace::SyscallDirection::ENTRY));
        EXPECT_CALL(mock_event_handler,
                    onSyscall(p_ptrace->getChildPid(),
                              write_syscall,
                              Ptrace::SyscallDirection::EXIT))
                .WillOnce(InvokeWithoutArgs(
                        [=]() {
                            Data received;
                            ASSERT_EQ(read(data_pipe[0],
                                           &received,
                                           sizeof(received)),
                                      (ssize_t) sizeof(received));
                            ASSERT_EQ(expected, received);
                        }
                ));
    }

    std::unique_ptr<Ptrace> p_ptrace;
    MockEventHandler        mock_event_handler;

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

        EXPECT_CALL(mock_event_handler,
                    onSyscall(_, _, _)).Times(AtLeast(10));

        const Syscall kill_syscall("kill");

        EXPECT_CALL(mock_event_handler,
                    onSyscall(_,
                              kill_syscall,
                              Ptrace::SyscallDirection::ENTRY));
        EXPECT_CALL(mock_event_handler,
                    onSyscall(_,
                              kill_syscall,
                              Ptrace::SyscallDirection::EXIT));

        const Syscall close_syscall("close");
        for (auto i = 0; i < 2; ++i) {
            EXPECT_CALL(mock_event_handler,
                        onSyscall(_,
                                  close_syscall,
                                  Ptrace::SyscallDirection::ENTRY));
            EXPECT_CALL(mock_event_handler,
                        onSyscall(_,
                                  close_syscall,
                                  Ptrace::SyscallDirection::EXIT));
        }

        openPipes();

        p_ptrace.reset(new Ptrace(args[0], args, mock_event_handler));

        closeUnusedPipeEnds();
    }

    InSequence in_sequence; // for making the constraints ordered

    int command_pipe[2];
    int data_pipe   [2];
};


TEST_F(PtraceTest, BasicTest) {
    sendCommand(0);

    const Syscall close_syscall("close");

    EXPECT_CALL(mock_event_handler,
                onSyscall(p_ptrace->getChildPid(),
                          close_syscall,
                          Ptrace::SyscallDirection::ENTRY));
    EXPECT_CALL(mock_event_handler,
                onSyscall(p_ptrace->getChildPid(),
                          close_syscall,
                          Ptrace::SyscallDirection::EXIT));

    EXPECT_CALL(mock_event_handler,
                onSyscall(p_ptrace->getChildPid(),
                          Syscall("exit_group"),
                          Ptrace::SyscallDirection::ENTRY));

    EXPECT_CALL(mock_event_handler, onExit(p_ptrace->getChildPid(), 0));

    p_ptrace->trace();
}

TEST_F(PtraceTest, MmapHijackTest) {
    sendCommand(1);

    EXPECT_CALL(mock_event_handler,
                onSyscall(p_ptrace->getChildPid(),
                          Syscall("mmap"),
                          Ptrace::SyscallDirection::ENTRY))
            .WillOnce(InvokeWithoutArgs([&]() {
                p_ptrace->pokeSyscall(Syscall("getpid"));
            }));
    EXPECT_CALL(mock_event_handler,
                onSyscall(p_ptrace->getChildPid(),
                          Syscall("getpid"),
                          Ptrace::SyscallDirection::EXIT));

    receiveData(p_ptrace->getChildPid());

    EXPECT_CALL(mock_event_handler,
                onSyscall(p_ptrace->getChildPid(),
                          Syscall("exit_group"),
                          Ptrace::SyscallDirection::ENTRY));

    EXPECT_CALL(mock_event_handler, onExit(p_ptrace->getChildPid(), 0));

    p_ptrace->trace();
}
