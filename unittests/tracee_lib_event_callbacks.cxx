//
// Created by mac on 1/7/19.
//

#include "tracee_lib_event_callbacks.h"

TraceeLibEventCallbacks::~TraceeLibEventCallbacks() = default;

void TraceeLibEventCallbacks::onStart    (pid_t pid){
    printf("TraceeLibEventCallbacks onStart %d\n", getpid());
}
void TraceeLibEventCallbacks::onExit     (pid_t pid, int retval){
    printf("TraceeLibEventCallbacks onExit %d\n", getpid());
}
void TraceeLibEventCallbacks::onSignal   (pid_t pid, int signal_num){
    printf("TraceeLibEventCallbacks onSignal %d\n", getpid());
}

void TraceeLibEventCallbacks::onSyscallEnter(pid_t pid, Ptrace::SyscallEnterAction& action){
    printf("TraceeLibEventCallbacks onSyscallEnter %d\n", getpid());
}
void TraceeLibEventCallbacks::onSyscallExit (pid_t pid, Ptrace::SyscallExitAction& action){
    printf("TraceeLibEventCallbacks onSyscallExit %d\n", getpid());
}