//
// Created by mac on 2/6/19.
//

#include "tracee_lib_test.h"

TraceeLibTest::TraceeLibTest() {
    char* args[] = {const_cast<char*>("./tracee_lib_tracee"), nullptr};
    p_ptrace.reset(new Ptrace(args[0], args, tracee_lib_event_callbacks));
}
void TraceeLibEventCallbacks::onStart    (pid_t pid){
    //printf("TraceeLibEventCallbacks onStart %d\n", getpid());
}
void TraceeLibEventCallbacks::onExit     (pid_t pid, int retval){
    //printf("TraceeLibEventCallbacks onExit %d\n", getpid());
}

void TraceeLibEventCallbacks::onSignal   (pid_t pid, int signal_num){
    //printf("TraceeLibEventCallbacks onSignal %d\n", getpid());
}
void TraceeLibEventCallbacks::onSyscallEnter(pid_t pid, Ptrace::SyscallEnterAction& action){
    // printf("TraceeLibEventCallbacks onSyscallEnter %d\n", getpid());
}

void TraceeLibEventCallbacks::onSyscallExit (pid_t pid, Ptrace::SyscallExitAction& action){
    //printf("TraceeLibEventCallbacks onSyscallExit %d\n", getpid());
}

void TraceeLibEventCallbacks::onTerminate(pid_t, int signal_num) {

}
/*
void TraceeLibTest::SetUp() {
    char* args[] = {const_cast<char*>("./tracee_lib_tracee"), nullptr};
    p_ptrace.reset(new Ptrace(args[0], args, tracee_lib_event_callbacks));
}
*/