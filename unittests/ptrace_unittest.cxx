#include <cassert>
#include <wait.h>
#include <memory>
#include <googletest/include/gtest/gtest.h>
#include "ptrace.h"


class PtraceTest : public ::testing::Test {
  protected:
    PtraceTest() : kill_syscall("kill"), command_pipe{-1, -1}, data_pipe{-1, -1} {
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

        static const Syscall read_syscall("read");
        for (int i = 0; i < 2; ++i) {
            ASSERT_EQ(read_syscall, p_ptrace->runUntilSyscallGate().first);
        }
    }

    template <typename Data>
    void receiveData(Data& data) {
        static const Syscall write_syscall("write");
        for (int i = 0; i < 2; ++i) {
            ASSERT_EQ(write_syscall, p_ptrace->runUntilSyscallGate().first);
        }

        ASSERT_EQ(read(data_pipe[0], &data, sizeof(data)), (ssize_t)sizeof(data));
    }

    std::unique_ptr<Ptrace> p_ptrace;
    const Syscall           kill_syscall;

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

        openPipes();

        p_ptrace.reset(new Ptrace(args[0], args));

        closeUnusedPipeEnds();

        auto max_tries = 1000;
        while (max_tries--) {
            auto syscall_info = p_ptrace->runUntilSyscallGate();
            ASSERT_EQ(syscall_info.second, Ptrace::SyscallDirection::ENTRY);

            syscall_info = p_ptrace->runUntilSyscallGate();
            ASSERT_EQ(syscall_info.second, Ptrace::SyscallDirection::EXIT);

            if (syscall_info.first == kill_syscall) {
                break;
            }
        }

        ASSERT_LE(0, max_tries) << kill_syscall << " wasn't found in the first " << max_tries << " syscalls";

        const Syscall close_syscall("close");
        for (int i = 0; i < 4; ++i) {
            ASSERT_EQ(close_syscall, p_ptrace->runUntilSyscallGate().first);
        }
    }

    int command_pipe[2];
    int data_pipe   [2];
};


TEST_F(PtraceTest, BasicTest) {
    sendCommand(0);

    auto syscall_info = p_ptrace->runUntilSyscallGate();
    EXPECT_EQ(std::string(syscall_info.first.toString()) , "close");
    ASSERT_EQ(syscall_info.second , Ptrace::SyscallDirection::ENTRY);

    syscall_info = p_ptrace->runUntilSyscallGate();
    EXPECT_EQ(std::string(syscall_info.first.toString()) , "close");
    ASSERT_EQ(syscall_info.second , Ptrace::SyscallDirection::EXIT);

    syscall_info = p_ptrace->runUntilSyscallGate();
    EXPECT_EQ(std::string(syscall_info.first.toString()) , "exit_group");
    EXPECT_EQ(syscall_info.second , Ptrace::SyscallDirection::ENTRY);

    EXPECT_THROW(syscall_info = p_ptrace->runUntilSyscallGate(), std::system_error);
}

TEST_F(PtraceTest, MmapHijackTest) {
    sendCommand(1);

    auto syscall_info = p_ptrace->runUntilSyscallGate();
    EXPECT_EQ(std::string(syscall_info.first.toString()) , "mmap");
    ASSERT_EQ(syscall_info.second , Ptrace::SyscallDirection::ENTRY);

    // modify mmap to return getpid()
    Syscall getpid_syscall("getpid");
    p_ptrace->pokeSyscall(getpid_syscall);

    syscall_info = p_ptrace->runUntilSyscallGate();
    EXPECT_EQ(syscall_info.first , getpid_syscall);
    ASSERT_EQ(syscall_info.second , Ptrace::SyscallDirection::EXIT);

    pid_t res;
    receiveData(res);
    ASSERT_EQ(p_ptrace->getChildPid(), res);

    auto exit_status = p_ptrace->runUntilExit();
    ASSERT_TRUE(exit_status.second);
    EXPECT_EQ(0, exit_status.first);
}
