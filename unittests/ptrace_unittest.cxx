#include <cassert>
#include <wait.h>
#include <googletest/include/gtest/gtest.h>
#include "ptrace.h"


TEST(PtraceTest, BasicTest) {
    char* args[] = {const_cast<char*>("./tracee"), nullptr};

    Ptrace ptrace("./tracee", args);

    decltype(ptrace.runUntilSyscallGate()) syscall_info;

    Syscall getpid_syscall(39); // TODO add ability to construct by "getpid" string?

    while (syscall_info.first != getpid_syscall) {
        syscall_info = ptrace.runUntilSyscallGate();
        ASSERT_EQ(syscall_info.second, Ptrace::SyscallDirection::ENTRY);

        syscall_info = ptrace.runUntilSyscallGate();
        ASSERT_EQ(syscall_info.second, Ptrace::SyscallDirection::EXIT);
    }

    syscall_info = ptrace.runUntilSyscallGate();
    ASSERT_EQ(std::string(syscall_info.first) , "close");
    ASSERT_EQ(syscall_info.second , Ptrace::SyscallDirection::ENTRY);

    syscall_info = ptrace.runUntilSyscallGate();
    ASSERT_EQ(std::string(syscall_info.first) , "close");
    ASSERT_EQ(syscall_info.second , Ptrace::SyscallDirection::EXIT);

    syscall_info = ptrace.runUntilSyscallGate();
    ASSERT_EQ(std::string(syscall_info.first) , "exit_group");
    ASSERT_EQ(syscall_info.second , Ptrace::SyscallDirection::ENTRY);

    ASSERT_THROW(syscall_info = ptrace.runUntilSyscallGate(), std::system_error);
}
