// C++ headers
#include <iostream>

// C headers
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>

#include "syscallents.h"

using std::cout;
using std::endl;

#define offsetof(type, member)  __builtin_offsetof (type, member)
#define get_tracee_reg(child_id, reg_name) _get_reg(child_id, offsetof(struct user, regs.reg_name))

long _get_reg(pid_t child_id, int offset) {
    long res = ptrace(PTRACE_PEEKUSER, child_id, offset);
    //assert(errno == 0);
    return res;
}


int callSafeSyscall(int syscall_return_value, int code_line) {
    if (syscall_return_value < 0) {
        printf("Failed on line %d with error %d - %s\n", code_line,
               syscall_return_value, strerror(errno));
    }
    return syscall_return_value;
}

#define SAFE_SYSCALL(syscall) \
    callSafeSyscall(syscall, __LINE__)

#define PTRACE_O_TRACESYSGOOD_MASK  0x80

enum class TraceeStatus {
    EXITED,
    TERMINATED,
    SIGNALED,
    SYSCALLED,
    CONTINUED
};

pid_t waitForDescendant(TraceeStatus& tracee_status) {
    int status;
    pid_t waited_pid = wait(&status);

    if (waited_pid < 0) {
        assert(errno == ECHILD);
        cout << "No more descendants to wait for!" << endl;
        return waited_pid;
    } // else, one of the descendants changed state

    cout << "Process #" << waited_pid << " ";

    if (WIFEXITED(status)) {
        tracee_status = TraceeStatus::EXITED;
        cout << "exited normally with value " << WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        tracee_status = TraceeStatus::TERMINATED;
        int signal_num = WTERMSIG(status);
        char* signal_name = strsignal(signal_num);
        cout << "terminated by " << signal_name << " (#" << signal_num << ")";
    } else if (WIFSTOPPED(status)) {
        int signal_num = WSTOPSIG(status);
        if (signal_num & PTRACE_O_TRACESYSGOOD_MASK) {
            // TODO: if-statement should be (signal_num == SIGTRAP|0x80)??
            // and should we use PTRACE_GETSIGINFO to obtain signal_num?? (continue reading man)
            // TODO: make sure it's sufficient to recognize system-call-stop
            tracee_status = TraceeStatus::SYSCALLED;
            int syscall_num = get_tracee_reg(waited_pid, orig_rax);
            cout << "syscalled with " << syscalls[syscall_num];
        } else {
            tracee_status = TraceeStatus::SIGNALED;
            char* signal_name = strsignal(signal_num);
            cout << "stopped by " << signal_name << " (#" << signal_num << ")";
        }
    } else if (WIFCONTINUED(status)) {
        assert(0); // we shouldn't get here if we use wait()
        // only waitpid() may return it if (options|WCONTINUED == WCONTINUED)
        tracee_status = TraceeStatus::CONTINUED;
        cout << "continued";
    }
    cout << endl;
    return waited_pid;
}

int main() {
  cout << "Tracer process id = " << getpid() << endl;
  pid_t child_pid = SAFE_SYSCALL(fork());    
  
  if (child_pid == 0) { // child process
    cout << "Tracee process id = " << getpid() << endl;
    char* args[] = {const_cast<char*>("date"), NULL};
    
    SAFE_SYSCALL(ptrace(PTRACE_TRACEME, 0, NULL, NULL));

    //SAFE_SYSCALL(execv("/bin/date", args));
	execv("./child_getpid", args);

  } else { // father process
    
    bool in_kernel = false;
    TraceeStatus tracee_status;	
    
    // for the first time, make sure the child stopped before execv
    pid_t descendant_pid = waitForDescendant(tracee_status);
    assert(descendant_pid == child_pid);
    assert(tracee_status == TraceeStatus::SIGNALED);

    // TODO: remove
    cout << "RAX " << get_tracee_reg(child_pid, orig_rax) 
            << " which is " << syscalls[get_tracee_reg(child_pid, orig_rax)] <<endl;
    in_kernel = !in_kernel;
    
    // after the child has stopped, we can now set the correct options
    SAFE_SYSCALL(ptrace(PTRACE_SETOPTIONS, child_pid, NULL, PTRACE_O_TRACESYSGOOD));
        
    while (1) {
      
      // TODO: remove - core dump
      /* 
        cout << "RAX before messing with orig_rax " << get_tracee_reg(child_pid, orig_rax) 
            << " which is " << syscalls[get_tracee_reg(child_pid, orig_rax)] <<endl;
        cout << "EBX " << get_tracee_reg(child_pid, rbx) << endl;
        cout << "ECX " << get_tracee_reg(child_pid, rcx) << endl;
        cout << "EDX " << get_tracee_reg(child_pid, rdx) << endl;
      */  
        //SAFE_SYSCALL(ptrace(PTRACE_POKEUSER, child_pid, offsetof(struct user, regs.orig_rax), 25));
        
        //cout << "RAX before continue " << get_tracee_reg(child_pid, orig_rax) << endl;
      // end remove
        SAFE_SYSCALL(ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL));

        pid_t descendant_pid = waitForDescendant(tracee_status);

        in_kernel = !in_kernel;
      
        if (descendant_pid < 0) { // no more descendants
	       break; 
        } // else, stop at the next system call entry or exit
        cout << descendant_pid << " " << (in_kernel ? "exits kernel" : "enters kernel") << endl;
        }
    }
  return 0;
}

