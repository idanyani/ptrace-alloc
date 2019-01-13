//
// Created by mac on 1/7/19.
//

#include "tracee_lib_event_callbacks.h"

TraceeLibEventCallbacks::~TraceeLibEventCallbacks() = default;

void TraceeLibEventCallbacks::onStart    (pid_t pid){
    printf("TraceeLibEventCallbacks onStart\n");
}
void TraceeLibEventCallbacks::onExit     (pid_t pid, int retval){
    printf("TraceeLibEventCallbacks onExit\n");
}
void TraceeLibEventCallbacks::onSignal   (pid_t pid, int signal_num){
    printf("TraceeLibEventCallbacks onSignal\n");
}

void TraceeLibEventCallbacks::onSyscallEnter(pid_t pid, Ptrace::SyscallEnterAction& action){
    printf("TraceeLibEventCallbacks onSyscallEnter\n");
}
void TraceeLibEventCallbacks::onSyscallExit (pid_t pid, Ptrace::SyscallEnterAction& action){
    printf("TraceeLibEventCallbacks onSyscallExit\n");
}