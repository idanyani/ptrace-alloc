#include <cassert>
#include <wait.h>
#include "ptrace.h"


int main() {
    char* args[] = {const_cast<char*>("./tracee"), nullptr};

    Ptrace ptrace("./tracee", args);

    decltype(ptrace.runUntilSyscallGate()) syscall_info;

    Syscall getpid_syscall(39); // TODO add ability to construct by "getpid" string?

    while (syscall_info.first != getpid_syscall) {
        syscall_info = ptrace.runUntilSyscallGate();
        assert(syscall_info.second == Ptrace::SyscallDirection::ENTRY);

        syscall_info = ptrace.runUntilSyscallGate();
        assert(syscall_info.second == Ptrace::SyscallDirection::EXIT);
    }

    syscall_info = ptrace.runUntilSyscallGate();
    assert(std::string(syscall_info.first) == "close");
    assert(syscall_info.second == Ptrace::SyscallDirection::ENTRY);

    syscall_info = ptrace.runUntilSyscallGate();
    assert(std::string(syscall_info.first) == "close");
    assert(syscall_info.second == Ptrace::SyscallDirection::EXIT);

    syscall_info = ptrace.runUntilSyscallGate();
    assert(std::string(syscall_info.first) == "exit_group");
    assert(syscall_info.second == Ptrace::SyscallDirection::ENTRY);

    bool thrown = false;
    try {
        syscall_info = ptrace.runUntilSyscallGate();
    } catch (const std::exception&) {
        thrown = true;
    }

    assert(thrown);
}
