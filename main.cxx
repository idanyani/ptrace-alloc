// C++ headers
#include <iostream>
using namespace std;

// C headers
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>

//TODO: fix this function and the following macro,
// and then replace all system call invocations in the code
int callSafeSyscall(int syscall_return_value, int code_line) {
	if (syscall_return_value != 0) {
		printf("Failed on line %d with error %d - %s\n", code_line, 
		syscall_return_value, strerror(syscall_return_value));
	}
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
            tracee_status = TraceeStatus::SYSCALLED;
            cout << "syscalled";
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
    // pid_t child_pid = SAFE_SYSCALL(fork());
    pid_t child_pid = fork();
    if (child_pid == 0) { // child process
        cout << "Tracee process id = " << getpid() << endl;
        char* args[] = {const_cast<char*>("date"), NULL};
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execv("/bin/date", args);
    } else { // father process
        TraceeStatus tracee_status;
        // for the first time, make sure the child stopped before execv
        pid_t descendant_pid = waitForDescendant(tracee_status);
        assert(descendant_pid == child_pid);
        assert(tracee_status == TraceeStatus::SIGNALED);
        // after the child has stopped, we can now set the correct options
        ptrace(PTRACE_SETOPTIONS, child_pid, NULL, PTRACE_O_TRACESYSGOOD);
        // and let the child resume
        ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL);
        while (1) {
            pid_t descendant_pid = waitForDescendant(tracee_status);
            if (descendant_pid < 0) { // no more descendants
                break;
            } // else, stop at the next system call entry or exit
            ptrace(PTRACE_SYSCALL, descendant_pid, NULL, NULL);
        }
    }
    return 0;
}
