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
        : event_callbacks_(event_handler) {

    pid_t tracee_pid = SAFE_SYSCALL(fork());

    if (tracee_pid == 0) { // child process

        SAFE_SYSCALL(ptrace(PTRACE_TRACEME, 0, NULL, NULL));
        SAFE_SYSCALL(execv(executable.c_str(), args));
        assert(false); // can't get here, SAFE_SYSCALL will throw on error
    }
}

Ptrace::~Ptrace() {
    for (auto process : process_list_) {
        kill(process.pid(), SIGKILL);
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
        int signal_to_inject = 0;

        auto process_iter = process_list_.find(TracedProcess(waited_pid)); // TODO is there a way not to create TracedProcess? (search by pid)
        if (process_iter != process_list_.end()) {
            if (WIFEXITED(status)) {
                int retval = WEXITSTATUS(status);

                logger_ << "exited normally with value " << retval << Logger::endl;
                process_list_.erase(process_iter);
                event_callbacks_.onExit(waited_pid, retval);
                continue;

            } else if (WIFSIGNALED(status)) {
                int signal_num = WTERMSIG(status);
                char* signal_name = strsignal(signal_num);

                logger_ << "terminated by \"" << signal_name << "\" (#" << signal_num << ")"
                        << Logger::endl;
                process_list_.erase(process_iter);
                event_callbacks_.onTerminate(waited_pid, signal_num);
                continue;
            }

            assert(WIFSTOPPED(status));
            int signal_num = WSTOPSIG(status);

            if (signal_num & PTRACE_O_TRACESYSGOOD_MASK) {
                assert(signal_num == (SIGTRAP | PTRACE_O_TRACESYSGOOD_MASK));

                process_iter->toggleKernelUser();

                logger_ << "syscalled"
                        // << " with \"" << getSyscall(*process_iter)
                        << "\"; " << (process_iter->isInsideKernel() ? "Enter" : "Exit") << Logger::endl;


                if (process_iter->isInsideKernel()) {
                    SyscallEnterAction action(*this, *process_iter);
                    event_callbacks_.onSyscallEnter(waited_pid, action);
                } else {
                    SyscallExitAction action(*this, *process_iter);
                    event_callbacks_.onSyscallExit(waited_pid, action);
                }

            } else if (signal_num == SIGTRAP) {
                const auto event = status >> 16; // status>>8 == (SIGTRAP | PTRACE_EVENT_foo << 8)
                logger_ << "trapped. Event: " << event;

                switch (event) {

                    case PTRACE_EVENT_FORK:
                    case PTRACE_EVENT_VFORK:
                    case PTRACE_EVENT_CLONE:
                        logger_ << " (fork/clone)" << Logger::endl;
                        break;
                    case PTRACE_EVENT_EXEC:
                        throw std::logic_error("case is not implemented"); // TODO: add a callback
                    default:
                        break;
                }

            } else {
                char* signal_name = strsignal(signal_num);

                logger_ << "signalled with \"" << signal_name << "\" (#" << signal_num << ")"
                        << Logger::endl;
                event_callbacks_.onSignal(waited_pid, signal_num);

                signal_to_inject = signal_num;
            }

        } else {
            // newborn process
            process_list_.emplace(waited_pid);

            logger_ << "is first traced" << Logger::endl;
            event_callbacks_.onStart(waited_pid);

            // after the newborn has stopped, we can now set the correct options
            auto flags = PTRACE_O_TRACESYSGOOD |
                         PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE;
            SAFE_SYSCALL(ptrace(PTRACE_SETOPTIONS, waited_pid, NULL, flags));
        }

        SAFE_SYSCALL(ptrace(PTRACE_SYSCALL, waited_pid, NULL, signal_to_inject));
    }
}

void Ptrace::setSyscall(const TracedProcess& tracee, Syscall syscall_to_run) {
    assert(process_list_.find(tracee) != process_list_.end());
    assert(tracee.isInsideKernel());
    SAFE_SYSCALL(ptrace(PTRACE_POKEUSER, tracee.pid(),
                        REG_OFFSET(orig_rax), syscall_to_run.getSyscallNum()));
}

void Ptrace::setReturnValue(const TracedProcess& tracee, long return_value) {
    assert(process_list_.find(tracee) != process_list_.end());
    assert(!tracee.isInsideKernel());
    SAFE_SYSCALL(ptrace(PTRACE_POKEUSER, tracee.pid(), REG_OFFSET(rax), return_value));
}

Syscall Ptrace::getSyscall(const TracedProcess& tracee) {
    assert(process_list_.find(tracee) != process_list_.end());
    return Syscall(static_cast<int>(SAFE_SYSCALL_BY_ERRNO(ptrace(PTRACE_PEEKUSER,
                                                                 tracee.pid(),
                                                                 REG_OFFSET(orig_rax)))));
}

Syscall Ptrace::SyscallAction::getSyscall() const {
    return ptrace_.getSyscall(tracee_);
}

void Ptrace::SyscallEnterAction::setSyscall(Syscall syscall) {
    ptrace_.setSyscall(tracee_, syscall);
}

void Ptrace::SyscallExitAction::setReturnValue(long return_value) {
    ptrace_.setReturnValue(tracee_, return_value);
}
