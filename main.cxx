// C++ headers
#include <iostream>
#include <vector>
#include <tuple>
#include <sstream>

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
using std::tuple;
using std::vector;


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
 
// TODO: change to generic system call
void printMmap(pid_t child_pid)
{
    cout << std::showbase << std::hex;
    cout << "Arguments to mmap: " << endl;
    cout << "1 : RDI = " << get_tracee_reg(child_pid, rdi) << endl; 
    cout << "2 : RSI = " << get_tracee_reg(child_pid, rsi) << endl;
    cout << "3 : RDX = " << get_tracee_reg(child_pid, rdx) << endl; 
    cout << "4 : R10 = " << get_tracee_reg(child_pid, r10) << endl;
    cout << "5 : R8 = " << get_tracee_reg(child_pid, r8) << endl;  
    cout << "6 : R9 = " << get_tracee_reg(child_pid, r9) << endl;  
                
    cout << std::noshowbase << std::dec; 
}

#define PTRACE_O_TRACESYSGOOD_MASK  0x80

enum class TraceeStatus {
    EXITED,
    TERMINATED,
    SIGNALED,
    SYSCALLED,
    CONTINUED
};
/*
TODO: add function that identifies systemcall to poke
TODO : add number of system call we are about to poke?
*/
bool sysCallPoke(pid_t child_pid, vector<tuple<bool, long>>& args)
{
    int arg_regs[6] = {offsetof(struct user, regs.rbx), offsetof(struct user, regs.rcx), 
                        offsetof(struct user, regs.rdx), offsetof(struct user, regs.rsi), 
                        offsetof(struct user, regs.rdi), offsetof(struct user, regs.rbp)};
    bool poke = true;
    for(int arg_i = 0; arg_i < 6; arg_i++)
    {
        if(std::get<0>(args[arg_i]))
            poke = poke && (_get_reg(child_pid, arg_regs[arg_i]) == std::get<1>(args[arg_i]));
    }        
    return poke;
}

pid_t waitForDescendant(TraceeStatus& tracee_status, bool poke_syscall, int new_syscall, vector<tuple<bool, long>>& args) {
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
            cout << "syscalled with " << syscalls[syscall_num] << endl ;
            // print mmap
            // not sure it's enough
            if(syscall_num == 9){ // TODO: change to parameter
                printMmap(waited_pid);
                // TODO : fix 
                if( /*(get_tracee_reg(waited_pid, rdi) == 0) &&
                    (get_tracee_reg(waited_pid, rsi) == 4) && 
                    (get_tracee_reg(waited_pid, rdx) == 3)*/ poke_syscall && sysCallPoke(waited_pid, args))
                        // TODO: move to unit test : Change to getpid
                        SAFE_SYSCALL(ptrace(PTRACE_POKEUSER, waited_pid, offsetof(
                                 struct user, regs.orig_rax), new_syscall));
            }

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

int main(int argc, char* argv[]) {
    //assert(argc == 7);
    cout << "Tracer process id = " << getpid() << endl;
    /*
    vector<tuple<bool, long>> poke_syscall_args;
    for(int i = 0; i < argc; i++)
    {
        long param;
        bool valid_arg; 

        istringstream iss (argv[i]);
        
        
        if((iss >> param) == NULL){
            cerr << "Invalid argument " << argv[i] << endl;
            return 1; // ???
        }
        
        //iss >> param;
        valid_arg = (param != -1);
        poke_syscall_args.push_back(make_tuple(valid_arg, param));

    }
    */
    pid_t child_pid = SAFE_SYSCALL(fork());    
  
    if (child_pid == 0) { // child process
        cout << "Tracee process id = " << getpid() << endl;
        char* args[] = {const_cast<char*>("date"), NULL};
    
        SAFE_SYSCALL(ptrace(PTRACE_TRACEME, 0, NULL, NULL));

        //SAFE_SYSCALL(execv("/bin/date", args));
	    //execv("./child_getpid", args);
        execv("./child_mmap", args);

  } else { // father process
    
    bool in_kernel = false;
    TraceeStatus tracee_status;	
    
    vector<tuple<bool, long>> poke_syscall_args;

    // TODO: change to reading from command line arguments 
    poke_syscall_args.push_back(std::make_tuple(false, -1));
    poke_syscall_args.push_back(std::make_tuple(false, -1));
    poke_syscall_args.push_back(std::make_tuple(true, 3));
    poke_syscall_args.push_back(std::make_tuple(true, 4));
    poke_syscall_args.push_back(std::make_tuple(true, 0));
    poke_syscall_args.push_back(std::make_tuple(false, -1));       

    // for the first time, make sure the child stopped before execv
    pid_t descendant_pid = waitForDescendant(tracee_status, true, 39, poke_syscall_args);
    assert(descendant_pid == child_pid);
    assert(tracee_status == TraceeStatus::SIGNALED);

    in_kernel = !in_kernel;
    
    // after the child has stopped, we can now set the correct options
    SAFE_SYSCALL(ptrace(PTRACE_SETOPTIONS, child_pid, NULL, PTRACE_O_TRACESYSGOOD));
        
    while (1) {
      
            SAFE_SYSCALL(ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL));

            pid_t descendant_pid = waitForDescendant(tracee_status, true, 39, poke_syscall_args);

            in_kernel = !in_kernel;
      
            if (descendant_pid < 0) { // no more descendants
	           break; 
            } // else, stop at the next system call entry or exit
            cout << descendant_pid << " " << (in_kernel ? "exits kernel" : "enters kernel") << endl;
        }
    }
    return 0;
}

