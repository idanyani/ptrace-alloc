//
// Created by mac on 3/16/19.
//

#ifndef PTRACE_ALLOC_TRACEE_LIB_DEFINES_H
#define PTRACE_ALLOC_TRACEE_LIB_DEFINES_H

int traceeHandleSyscallReturnValue(int syscall_return_value, unsigned int code_line);

#define TRACEE_SAFE_SYSCALL(syscall) \
({int _ret_val = traceeHandleSyscallReturnValue(syscall, __LINE__); _ret_val;})

#endif //PTRACE_ALLOC_TRACEE_LIB_DEFINES_H