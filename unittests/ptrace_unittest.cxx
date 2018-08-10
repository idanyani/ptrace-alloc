#include <cassert>
#include <wait.h>
#include <googletest/include/gtest/gtest.h>
#include "ptrace.h"


TEST(PtraceTest, BasicTest) {
    char* args[] = {const_cast<char*>("./tracee"), nullptr};

    Ptrace ptrace(args[0], args);

    decltype(ptrace.runUntilSyscallGate()) syscall_info;

    Syscall kill_syscall("kill");

    while (syscall_info.first != kill_syscall) {
        syscall_info = ptrace.runUntilSyscallGate();
        ASSERT_EQ(syscall_info.second, Ptrace::SyscallDirection::ENTRY);

        syscall_info = ptrace.runUntilSyscallGate();
        ASSERT_EQ(syscall_info.second, Ptrace::SyscallDirection::EXIT);
    }

    syscall_info = ptrace.runUntilSyscallGate();
    EXPECT_EQ(std::string(syscall_info.first.syscallToString()) , "close");
    ASSERT_EQ(syscall_info.second , Ptrace::SyscallDirection::ENTRY);

    syscall_info = ptrace.runUntilSyscallGate();
    EXPECT_EQ(std::string(syscall_info.first.syscallToString()) , "close");
    ASSERT_EQ(syscall_info.second , Ptrace::SyscallDirection::EXIT);

    syscall_info = ptrace.runUntilSyscallGate();
    EXPECT_EQ(std::string(syscall_info.first.syscallToString()) , "exit_group");
    EXPECT_EQ(syscall_info.second , Ptrace::SyscallDirection::ENTRY);

    EXPECT_THROW(syscall_info = ptrace.runUntilSyscallGate(), std::system_error);
}

TEST(PtraceTest, MmapHijackTest) {
    char* args[] = {const_cast<char*>("./tracee_mmap"), nullptr};

    Ptrace ptrace(args[0], args);

    decltype(ptrace.runUntilSyscallGate()) syscall_info;

    Syscall kill_syscall("kill");

    while (syscall_info.first != kill_syscall) {
        syscall_info = ptrace.runUntilSyscallGate();
        ASSERT_EQ(syscall_info.second, Ptrace::SyscallDirection::ENTRY);

        syscall_info = ptrace.runUntilSyscallGate();
        ASSERT_EQ(syscall_info.second, Ptrace::SyscallDirection::EXIT);
    }

    syscall_info = ptrace.runUntilSyscallGate();
    EXPECT_EQ(std::string(syscall_info.first.syscallToString()) , "mmap");
    ASSERT_EQ(syscall_info.second , Ptrace::SyscallDirection::ENTRY);

    // modify mmap to return getpid()
    Syscall getpid_syscall("getpid");
    ptrace.pokeSyscall(getpid_syscall);

    syscall_info = ptrace.runUntilSyscallGate();
    EXPECT_EQ(syscall_info.first , getpid_syscall);
    ASSERT_EQ(syscall_info.second , Ptrace::SyscallDirection::EXIT);

    auto exit_status = ptrace.runUntilExit();
    ASSERT_TRUE(exit_status.second);
    EXPECT_EQ(0, exit_status.first);
}
