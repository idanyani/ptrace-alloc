#include "ptrace.h"

#include <sys/user.h> // struct user
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h> // fork/execv
#include <cassert> // assert
#include <csignal>
#include <cstddef> // offsetof
#include <cstring> // strsignal
#include <system_error> // std::system_error


using std::string;

template<typename T>
T handleSyscallReturnValue(T syscall_return_value, unsigned code_line) {
    if (syscall_return_value < 0) {
        throw std::system_error(errno,
                                std::system_category(),
                                string("Failed on line:") + std::to_string(code_line));
    }
    return syscall_return_value;
}

#define SAFE_SYSCALL(syscall) \
    handleSyscallReturnValue(syscall, __LINE__)

#define SAFE_SYSCALL_BY_ERRNO(syscall)                      \
    ({                                                      \
        errno = 0;                                          \
        const auto ret = syscall;                           \
        handleSyscallReturnValue(-errno, __LINE__);         \
        ret;                                                \
    }) // https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html


#define REG_OFFSET(reg_name_)       offsetof(struct user, regs.reg_name_)

#define PTRACE_O_TRACESYSGOOD_MASK  0x80


Ptrace::Ptrace(const std::string& executable, char* args[], EventCallbacks& event_handler)
        : event_callbacks_(event_handler), tracee_pid_(0), in_kernel_(false) {
    tracee_pid_ = SAFE_SYSCALL(fork());

    if (tracee_pid_== 0) { // child process

        SAFE_SYSCALL(ptrace(PTRACE_TRACEME, 0, NULL, NULL));
        SAFE_SYSCALL(execv(executable.c_str(), args));
        assert(false); // can't get here, SAFE_SYSCALL will throw on error
    }

    logger_ << "Tracee process id = " << tracee_pid_ << Logger::endl;

    int status;
    const pid_t descendant_pid = wait(&status);
    assert(descendant_pid == tracee_pid_); //TODO: will fail on grandchildren tracing
    assert(WIFSTOPPED(status));

    // after the child has stopped, we can now set the correct options
    SAFE_SYSCALL(ptrace(PTRACE_SETOPTIONS, tracee_pid_, NULL, PTRACE_O_TRACESYSGOOD));
}

Ptrace::~Ptrace() {
    if (tracee_pid_) {
        kill(tracee_pid_, SIGKILL);
    }
}

void Ptrace::startTracing() {
    int signal_num = 0;
    while (true) {
        SAFE_SYSCALL(ptrace(PTRACE_SYSCALL, tracee_pid_, NULL, signal_num));

        int status;
        pid_t waited_pid = wait(&status);
        assert(waited_pid == tracee_pid_); //TODO: will fail on grandchildren tracing
        if (waited_pid < 0) {
            assert(errno == ECHILD);
            break;
        } // else, one of the descendants changed state

        logger_ << "Process #" << waited_pid << " ";

        if (WIFEXITED(status)) {
            int retval = WEXITSTATUS(status);

            logger_ << "exited normally with value " << retval << Logger::endl;
            event_callbacks_.onExit(waited_pid, retval);
            return;
        } else if (WIFSIGNALED(status)) {
            signal_num = WTERMSIG(status);
            char* signal_name = strsignal(signal_num);

            logger_ << "terminated by " << signal_name << " (#" << signal_num << ")" << Logger::endl;
            event_callbacks_.onTerminate(waited_pid, signal_num);
            return;
        } else if (WIFSTOPPED(status)) {
            signal_num = WSTOPSIG(status);
            if (signal_num & PTRACE_O_TRACESYSGOOD_MASK) {
                assert(signal_num == (SIGTRAP | PTRACE_O_TRACESYSGOOD_MASK));

                Syscall syscall(static_cast<int>(SAFE_SYSCALL_BY_ERRNO(ptrace(PTRACE_PEEKUSER,
                                                                              waited_pid,
                                                                              REG_OFFSET(orig_rax)))));

                in_kernel_ = !in_kernel_;
                
                if (in_kernel_) {
                    event_callbacks_.onSyscallEnter(waited_pid, syscall);
                } else {
                    event_callbacks_.onSyscallExit(waited_pid, syscall);
                }

                logger_ << "syscalled with \"" << syscall << "\"; " << (in_kernel_ ? "Enter" : "Exit") << Logger::endl;
                signal_num = 0;
            } else {
                char* signal_name = strsignal(signal_num);

                logger_ << "stopped by " << signal_name << " (#" << signal_num << ")" << Logger::endl;
                event_callbacks_.onSignal(waited_pid, signal_num);
            }
        } else if (WIFCONTINUED(status)) {
            assert(0); // we shouldn't get here if we use wait()
            // only waitpid() may return it if (options|WCONTINUED == WCONTINUED)
            logger_ << "continued" << Logger::endl;
        }
    }
}

void Ptrace::pokeSyscall(const Syscall& syscall_to_run) {
    assert(in_kernel_);
    SAFE_SYSCALL(ptrace(PTRACE_POKEUSER, tracee_pid_,
                        offsetof(struct user, regs.orig_rax), syscall_to_run.getSyscallNum()));
}