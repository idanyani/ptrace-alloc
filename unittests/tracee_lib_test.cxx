//
// Created by mac on 2/6/19.
//

#include "tracee_lib_test.h"

void TraceeLibMockEventCallbacks::onSyscallEnter(pid_t pid, Ptrace::SyscallEnterAction& action) {

    onSyscallEnterT(pid, action);
}

int TraceeLibMockEventCallbacks::onSyscallExitInner(pid_t pid, Ptrace::SyscallExitAction& action) {
    onSyscallExitT(pid, action);
    return 0;
}


std::unique_ptr<Ptrace>
        initPtrace(char** args, TraceeLibMockEventCallbacks& event_callbacks) {
    std::unique_ptr<Ptrace> p_ptarce;

    p_ptarce.reset(new Ptrace(args[0], args, event_callbacks, true));
    return p_ptarce;
}
