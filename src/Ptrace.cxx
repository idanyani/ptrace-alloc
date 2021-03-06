#include "Ptrace.h"

#include <sys/user.h> // struct user
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h> // fork/execv
#include <cassert> // assert
#include <csignal>
#include <cstddef> // offsetof
#include <cstring> // strsignal
#include <system_error> // std::system_error
#include <stdlib.h> // putenv
#include "../TraceeLib/tracee_lib.h"
#include <sys/stat.h> //mkdir
#include <sys/types.h> //mkdir

using std::string;


#define REG_OFFSET(reg_name_)       offsetof(struct user, regs.reg_name_)

#define PTRACE_O_TRACESYSGOOD_MASK  0x80

#define IS_EVENT(status_, event_) status_>> 8 == (SIGTRAP | event_ << 8)


Ptrace::Ptrace(const std::string& executable, char* args[], EventCallbacks& event_handler, bool load_lib)
        : event_callbacks_(event_handler), load_lib_(load_lib) {
    // TODO: add path to tracee library as an argument
    // FIXME: do we need to do LD_PRELOAD trick after execve for the tracee?
    if(load_lib_) {
        char env_var[] = "LD_PRELOAD=/home/mac/CLionProjects/ptrace-alloc/cmake-build-debug/TraceeLib/libtracee_l.so";
        putenv(env_var);

        setUserSignals();  // set up user signal handlers for tracer

        try {
            SAFE_SYSCALL(mkdir("/tmp/ptrace_fifo", 0777)); // create directory for tracee fifo's
        } catch(const std::system_error& e){
            if(e.code().value() != EEXIST) // if /tmp/ptrace_fifo already exists, resume the execution as normal
                throw e;
        }
    }

    pid_t tracee_pid = SAFE_SYSCALL(fork());

    if (tracee_pid == 0) { // child process

        SAFE_SYSCALL(ptrace(PTRACE_TRACEME, 0, NULL, NULL));
        SAFE_SYSCALL(execv(executable.c_str(), args));
        assert(false); // can't get here, SAFE_SYSCALL will throw on error
    }
}

Ptrace::~Ptrace() {
    //SAFE_SYSCALL(rmdir("/tmp/ptrace_fifo")); // FIXME: integrate boost FileSystem

    for (auto process : process_list_) {
        kill(process.first, SIGKILL);
    }
}

void Ptrace::startTracing() {

    while (true) {

        int   status;
        pid_t waited_pid = waitpid(-1,      // wait for any child
                                   &status,
                                   __WALL); // wait for all children (including 'clone'd)
        if (waited_pid < 0) {
            if (errno == EINTR) continue;
            assert(errno == ECHILD);
            break;
        } // else, one of the descendants changed state

        logger_ << "Process #" << waited_pid << " ";

        ProcessItr process_iter = process_list_.find(waited_pid);
        if (process_iter != process_list_.end()) {

            if (WIFEXITED(status)) {
                handleExitedProcess(status, process_iter);
                continue;

            } else if (WIFSIGNALED(status)) {
                handleTerminatedProcess(status, process_iter);
                continue;
            }

            assert(WIFSTOPPED(status));
            int signal_num = WSTOPSIG(status);
            //logger_ << "\nDBG : " << status << " " << strsignal(signal_num) << logger_.endl;
            if (isSyscallStop(signal_num))
                handleSyscalledProcess(status, signal_num, process_iter);
            else
                handleSignaledProcess(status, signal_num, process_iter);

        } else
            handleNewBornProcess(waited_pid);
    }
}

void Ptrace::setSyscall(const TracedProcess& tracee, Syscall syscall_to_run) {
    assert(process_list_.find(tracee.pid()) != process_list_.end());
    assert(tracee.isInsideKernel());
    SAFE_SYSCALL(ptrace(PTRACE_POKEUSER, tracee.pid(),
                        REG_OFFSET(orig_rax), syscall_to_run.getSyscallNum()));
}

void Ptrace::setReturnValue(const TracedProcess& tracee, long return_value) {
    assert(process_list_.find(tracee.pid()) != process_list_.end());
    assert(!tracee.isInsideKernel());
    SAFE_SYSCALL(ptrace(PTRACE_POKEUSER, tracee.pid(), REG_OFFSET(rax), return_value));
}

Syscall Ptrace::getSyscall(const TracedProcess& tracee) {
    assert(process_list_.find(tracee.pid()) != process_list_.end());
    return Syscall(static_cast<int>(SAFE_SYSCALL_BY_ERRNO(ptrace(PTRACE_PEEKUSER,
                                                                 tracee.pid(),
                                                                 REG_OFFSET(orig_rax)))));
}

Syscall Ptrace::SyscallAction::getSyscall() const {
    return ptrace_.getSyscall(tracee_);
}

const TracedProcess& Ptrace::SyscallAction::getTracee() const {
    return tracee_;
}


void Ptrace::SyscallEnterAction::setSyscall(Syscall syscall) {
    ptrace_.setSyscall(tracee_, syscall);
}

void Ptrace::SyscallExitAction::setReturnValue(long return_value) {
    ptrace_.setReturnValue(tracee_, return_value);
}

void Ptrace::setLoggerVerbosity(Logger::Verbosity verbosity){
    logger_ << verbosity;
}

bool Ptrace::isSyscallStop(int sig_num) {
    return sig_num == (SIGTRAP | 0x80);
}


void Ptrace::handleExitedProcess(int status, ProcessItr waited_process) {
    int retval = WEXITSTATUS(status);
    pid_t waited_pid = waited_process->first;

    logger_ << "exited normally with value " << retval << Logger::endl;
    process_list_.erase(waited_process);
    event_callbacks_.onExit(waited_pid, retval);
}

void Ptrace::handleTerminatedProcess(int status, ProcessItr waited_process) {
    int signal_num = WTERMSIG(status);
    char* signal_name = strsignal(signal_num);
    pid_t waited_pid = waited_process->first;

    logger_ << "terminated by \"" << signal_name << "\" (#" << signal_num << ")"
            << Logger::endl;
    process_list_.erase(waited_process);
    event_callbacks_.onTerminate(waited_pid, signal_num);
}

void Ptrace::handleSyscalledProcess(int status, int signal_num, ProcessItr waited_process) {
    pid_t waited_pid = waited_process->first;
    int signal_to_inject = 0;
    TracedProcess& process = waited_process->second;

    assert(signal_num == (SIGTRAP | PTRACE_O_TRACESYSGOOD_MASK));

    process.toggleKernelUser();

    if(!process.returningFromSignal()) {
        logger_ << "syscalled" << " with \"" << getSyscall(process)
                << "\"; " << (process.isInsideKernel() ? "Enter" : "Exit") << Logger::endl;
    } else {
        logger_ << "returned from signal" << Logger::endl;
    }

    if (process.isInsideKernel()) {
        SyscallEnterAction action(*this, process);
        signal_to_inject = event_callbacks_.onSyscallEnter(waited_pid, action);
    } else {
        SyscallExitAction action(*this, process);
        signal_to_inject = event_callbacks_.onSyscallExit(waited_pid, action);
    }

    if(startingReturnFromSignal(process))
        process.setReturningFromSignal(true);

    else if(finishingReturnFromSignal(process)){
        assert(process.returningFromSignal());
        process.setReturningFromSignal(false);
    }

    if(load_lib_){

        if(!process.isInsideKernel() && getSyscall(process) == Syscall("kill")){
            int kill_sig_num = static_cast<int>(SAFE_SYSCALL_BY_ERRNO(ptrace(PTRACE_PEEKUSER,
                                                                             waited_pid,
                                                                             REG_OFFSET(rsi))));
            if(kill_sig_num == 0) {
                process.setHasUserSignalHandlers(true);
                signal_to_inject = SIGUSR2;
            }
        }
    }
    SAFE_SYSCALL(ptrace(PTRACE_SYSCALL, waited_pid, NULL, signal_to_inject));
}

void Ptrace::handleSignaledProcess(int status, int signal_num, ProcessItr waited_process) {
    pid_t waited_pid = waited_process->first;
    char* signal_name = strsignal(signal_num);

    if(signal_num == SIGTRAP){
        handleTrappedProcess(status, signal_num, waited_process);
    } else {

        logger_ << "signalled with \"" << signal_name << "\" (#" << signal_num << ")"
                << " orig_rax " <<
                static_cast<int>(SAFE_SYSCALL_BY_ERRNO(ptrace(PTRACE_PEEKUSER,
                                                              waited_pid,
                                                              REG_OFFSET(orig_rax))))
                << Logger::endl;
        event_callbacks_.onSignal(waited_pid, signal_num);

        SAFE_SYSCALL(ptrace(PTRACE_SYSCALL, waited_pid, NULL, signal_num));
    }
}

void Ptrace::handleTrappedProcess(int status, int signal_num, Ptrace::ProcessItr waited_process) {
    pid_t waited_pid = waited_process->first;
    TracedProcess& process = waited_process->second;
    assert(signal_num == SIGTRAP);

    logger_ << "trapped: ";
    if(IS_EVENT(status, PTRACE_EVENT_FORK)) {
        event_callbacks_.onFork(waited_pid);
        logger_ << "fork" << logger_.endl;
    }
    else if(IS_EVENT(status, PTRACE_EVENT_EXEC)) {
        process.setHasUserSignalHandlers(false);
        event_callbacks_.onExec(waited_pid);
        logger_ << "exec" << logger_.endl;
    }
    else if(IS_EVENT(status, PTRACE_EVENT_CLONE)){
        event_callbacks_.onClone(waited_pid);
        logger_ << "clone" << logger_.endl;
    }
    else if(IS_EVENT(status, PTRACE_EVENT_VFORK)){
        event_callbacks_.onVFork(waited_pid);
        logger_ << "vfork" << logger_.endl;
    }
    else if(IS_EVENT(status, PTRACE_EVENT_VFORK_DONE)){
        event_callbacks_.onVForkDone(waited_pid);
        logger_ << "vfork done" << logger_.endl;
    } else{
        event_callbacks_.onSignal(waited_pid, signal_num);
        logger_ << "received signal SIGTRAP" << logger_.endl;
    }

    SAFE_SYSCALL(ptrace(PTRACE_SYSCALL, waited_pid, NULL, signal_num));
}


void Ptrace::handleNewBornProcess(pid_t waited_pid) {

    TracedProcess newborn_process(waited_pid);

    process_list_.emplace(waited_pid, newborn_process);

    logger_ << "is first traced" << Logger::endl;
    event_callbacks_.onStart(waited_pid);

    // after the newborn has stopped, we can now set the correct options
    auto flags = PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK |
                 PTRACE_O_TRACEVFORK | PTRACE_O_TRACEVFORKDONE |
                 PTRACE_O_TRACECLONE | PTRACE_O_TRACEEXEC;
    SAFE_SYSCALL(ptrace(PTRACE_SETOPTIONS, waited_pid, NULL, flags));
    SAFE_SYSCALL(ptrace(PTRACE_SYSCALL, waited_pid, NULL, 0));
}

bool Ptrace::startingReturnFromSignal(const TracedProcess& process) {
    return (process.isInsideKernel() && getSyscall(process) == Syscall("rt_sigreturn"));
}

bool Ptrace::finishingReturnFromSignal(const TracedProcess& process) {
    int orig_rax = static_cast<int>(SAFE_SYSCALL_BY_ERRNO(ptrace(PTRACE_PEEKUSER,
                                                                 process.pid(),
                                                                 REG_OFFSET(orig_rax))));
    return (!process.isInsideKernel() && process.returningFromSignal() && orig_rax < 0);
}