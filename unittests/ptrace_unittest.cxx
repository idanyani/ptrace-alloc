#include <cassert>
#include <wait.h>
#include <memory>
#include <googletest/include/gtest/gtest.h>
#include "ptrace.h"


class PtraceTest : public ::testing::Test {
  protected:
    PtraceTest() : kill_syscall("kill") {}

    void start_tracee(const char* tracee_path) {
        char* args[] = {const_cast<char*>(tracee_path), nullptr};

        p_ptrace.reset(new Ptrace(args[0], args));

        const auto max_tries = 1000;
        for (auto i = 0; i < max_tries; ++i) {
            auto syscall_info = p_ptrace->runUntilSyscallGate();
            ASSERT_EQ(syscall_info.second, Ptrace::SyscallDirection::ENTRY);

            syscall_info = p_ptrace->runUntilSyscallGate();
            ASSERT_EQ(syscall_info.second, Ptrace::SyscallDirection::EXIT);

            if (syscall_info.first == kill_syscall)
                return;
        }

        FAIL() << kill_syscall << " wasn't found in the first " << max_tries << " syscalls";
    }

    std::unique_ptr<Ptrace> p_ptrace;
    const Syscall           kill_syscall;

};


TEST_F(PtraceTest, BasicTest) {
    start_tracee("./tracee");

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
    start_tracee("./tracee_mmap");

    auto syscall_info = p_ptrace->runUntilSyscallGate();
    EXPECT_EQ(std::string(syscall_info.first.toString()) , "mmap");
    ASSERT_EQ(syscall_info.second , Ptrace::SyscallDirection::ENTRY);

    // modify mmap to return getpid()
    Syscall getpid_syscall("getpid");
    p_ptrace->pokeSyscall(getpid_syscall);

    syscall_info = p_ptrace->runUntilSyscallGate();
    EXPECT_EQ(syscall_info.first , getpid_syscall);
    ASSERT_EQ(syscall_info.second , Ptrace::SyscallDirection::EXIT);

    auto exit_status = p_ptrace->runUntilExit();
    ASSERT_TRUE(exit_status.second);
    EXPECT_EQ(0, exit_status.first);
}
