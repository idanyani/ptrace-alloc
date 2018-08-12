#include "ptrace.h"

#include <cassert>
#include <cstring>
#include <system_error>

#include <csignal>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>

#include "syscall.h"

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


#define offsetof(type, member)              __builtin_offsetof (type, member)
#define get_tracee_reg(child_id, reg_name)  _get_reg(child_id, offsetof(struct user, regs.reg_name))

long _get_reg(pid_t child_id, int offset) {
    return SAFE_SYSCALL_BY_ERRNO(ptrace(PTRACE_PEEKUSER, child_id, offset));
}

#define PTRACE_O_TRACESYSGOOD_MASK  0x80


Ptrace::Ptrace(const std::string& executable, char* args[]) :
        tracee_pid_(0), in_kernel_(false) {
    tracee_pid_ = SAFE_SYSCALL(fork());

    if (tracee_pid_== 0) { // child process

        SAFE_SYSCALL(ptrace(PTRACE_TRACEME, 0, NULL, NULL));
        SAFE_SYSCALL(execv(executable.c_str(), args));
        assert(false); // can't get here, SAFE_SYSCALL will throw on error
    }

    logger_ << "Tracee process id = " << tracee_pid_ << Logger::endl;
    TraceeStatus tracee_status;
    pid_t descendant_pid = waitForDescendant(tracee_status);
    assert(descendant_pid == tracee_pid_);
    assert(tracee_status == TraceeStatus::SIGNALED);

    // after the child has stopped, we can now set the correct options
    SAFE_SYSCALL(ptrace(PTRACE_SETOPTIONS, tracee_pid_, NULL, PTRACE_O_TRACESYSGOOD));
}

Ptrace::~Ptrace() {
    if (tracee_pid_) {
        kill(tracee_pid_, SIGKILL);
    }
}

std::pair<Syscall, Ptrace::SyscallDirection> Ptrace::runUntilSyscallGate() {

    TraceeStatus tracee_status = TraceeStatus::SYSCALLED;
    int entry = 0;

    do {
        if (tracee_status != TraceeStatus::SIGNALED) {
            // preserve signal only when signal is sent
            entry = 0;
        }

        SAFE_SYSCALL(ptrace(PTRACE_SYSCALL, tracee_pid_, NULL, entry));

        auto descendant_pid = waitForDescendant(tracee_status, &entry);
        assert(descendant_pid == tracee_pid_);

    } while (tracee_status != TraceeStatus::SYSCALLED);


    // currently this condition is always true
    if (tracee_status == TraceeStatus::SYSCALLED) {
        in_kernel_ = !in_kernel_;
    }

    auto direction = in_kernel_ ? SyscallDirection::ENTRY : SyscallDirection::EXIT;

    return std::make_pair(Syscall(entry), direction);
}

std::pair<int, bool> Ptrace::runUntilExit() {
    if (in_kernel_) {// preserve "in_kernel_" validity
        runUntilSyscallGate();
    }

    TraceeStatus tracee_status = TraceeStatus::SYSCALLED;
    int entry = 0;

    do {
        if (tracee_status != TraceeStatus::SIGNALED) {
            // preserve signal only when signal is sent
            entry = 0;
        }

        SAFE_SYSCALL(ptrace(PTRACE_CONT, tracee_pid_, NULL, entry));

        auto descendant_pid = waitForDescendant(tracee_status, &entry);
        assert(descendant_pid == tracee_pid_);

    } while (tracee_status != TraceeStatus::EXITED && tracee_status != TraceeStatus::TERMINATED);

    return std::make_pair(entry, tracee_status == TraceeStatus::EXITED);
}

pid_t Ptrace::waitForDescendant(TraceeStatus& tracee_status, int* entry) {
    int status;
    pid_t waited_pid = wait(&status);
    if (waited_pid < 0) {
        assert(errno == ECHILD);
        throw NoChildren();
    } // else, one of the descendants changed state

    logger_ << "Process #" << waited_pid << " ";

    if (WIFEXITED(status)) {
        int retval = WEXITSTATUS(status);
        tracee_status = TraceeStatus::EXITED;

        if (entry) {
            *entry = retval;
        }

        logger_ << "exited normally with value " << retval;
    } else if (WIFSIGNALED(status)) {
        tracee_status = TraceeStatus::TERMINATED;
        int signal_num = WTERMSIG(status);
        char* signal_name = strsignal(signal_num);
        logger_ << "terminated by " << signal_name << " (#" << signal_num << ")";

        if (entry) {
            *entry = signal_num;
        }
    } else if (WIFSTOPPED(status)) {
        int signal_num = WSTOPSIG(status);
        if (signal_num & PTRACE_O_TRACESYSGOOD_MASK) {
            assert(signal_num == (SIGTRAP | PTRACE_O_TRACESYSGOOD_MASK));
            tracee_status = TraceeStatus::SYSCALLED;

            auto syscall_num = static_cast<int>(get_tracee_reg(waited_pid, orig_rax));
            logger_ << "syscalled with " << Syscall(syscall_num);

            if (entry) {
                *entry = syscall_num;
            }

        } else {
            tracee_status = TraceeStatus::SIGNALED;
            char* signal_name = strsignal(signal_num);
            logger_ << "stopped by " << signal_name << " (#" << signal_num << ")";

            if (entry) {
                *entry = signal_num;
            }
        }
    } else if (WIFCONTINUED(status)) {
        assert(0); // we shouldn't get here if we use wait()
        // only waitpid() may return it if (options|WCONTINUED == WCONTINUED)
        tracee_status = TraceeStatus::CONTINUED;
        logger_ << "continued";
    }
    logger_ << Logger::endl;
    return waited_pid;
}

void Ptrace::pokeSyscall(const Syscall& syscallToRun) {
    assert(in_kernel_);
    SAFE_SYSCALL(ptrace(PTRACE_POKEUSER, tracee_pid_,
                        offsetof(struct user, regs.orig_rax), syscallToRun.getSyscallNum()));
}