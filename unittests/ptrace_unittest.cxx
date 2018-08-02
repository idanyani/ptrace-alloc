#include <cassert>
#include <wait.h>
#include <googletest/include/gtest/gtest.h>
#include "ptrace.h"


TEST(PtraceTest, BasicTest) {
    char* args[] = {const_cast<char*>("./tracee"), nullptr};

    Ptrace ptrace(args[0], args);

    decltype(ptrace.runUntilSyscallGate()) syscall_info;

    Syscall getpid_syscall("getpid");

    while (syscall_info.first != getpid_syscall) {
        syscall_info = ptrace.runUntilSyscallGate();
        ASSERT_EQ(syscall_info.second, Ptrace::SyscallDirection::ENTRY);

        syscall_info = ptrace.runUntilSyscallGate();
        ASSERT_EQ(syscall_info.second, Ptrace::SyscallDirection::EXIT);
    }

    syscall_info = ptrace.runUntilSyscallGate();
    EXPECT_EQ(std::string(syscall_info.first) , "close");
    ASSERT_EQ(syscall_info.second , Ptrace::SyscallDirection::ENTRY);

    syscall_info = ptrace.runUntilSyscallGate();
    EXPECT_EQ(std::string(syscall_info.first) , "close");
    ASSERT_EQ(syscall_info.second , Ptrace::SyscallDirection::EXIT);

    syscall_info = ptrace.runUntilSyscallGate();
    EXPECT_EQ(std::string(syscall_info.first) , "exit_group");
    EXPECT_EQ(syscall_info.second , Ptrace::SyscallDirection::ENTRY);

    EXPECT_THROW(syscall_info = ptrace.runUntilSyscallGate(), std::system_error);
}

TEST(PtraceTest, DISABLED_MmapHijackTest) {
    char* args[] = {const_cast<char*>("./tracee_mmap"), nullptr};

    Ptrace ptrace(args[0], args);

    decltype(ptrace.runUntilSyscallGate()) syscall_info;

    Syscall getpid_syscall("getpid");

    while (syscall_info.first != getpid_syscall) {
        syscall_info = ptrace.runUntilSyscallGate();
        ASSERT_EQ(syscall_info.second, Ptrace::SyscallDirection::ENTRY);

        syscall_info = ptrace.runUntilSyscallGate();
        ASSERT_EQ(syscall_info.second, Ptrace::SyscallDirection::EXIT);
    }

    syscall_info = ptrace.runUntilSyscallGate();
    EXPECT_EQ(std::string(syscall_info.first) , "mmap");
    ASSERT_EQ(syscall_info.second , Ptrace::SyscallDirection::ENTRY);

    //TODO: modify mmap to return getpid()

    syscall_info = ptrace.runUntilSyscallGate();
    EXPECT_EQ(std::string(syscall_info.first) , "mmap");
    ASSERT_EQ(syscall_info.second , Ptrace::SyscallDirection::EXIT);

    auto exit_status = ptrace.runUntilExit();
    ASSERT_TRUE(exit_status.second);
    EXPECT_EQ(getpid(), exit_status.first);
}
