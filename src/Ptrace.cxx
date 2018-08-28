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

    process_list_.emplace(tracee_pid);
    logger_ << "Tracee process id = " << tracee_pid << Logger::endl;
}

Ptrace::~Ptrace() {
    for (auto process : process_list_) {
        kill(process.pid_, SIGKILL);
    }
}

static inline
void toggle(bool& var) {
    var = !var;
}

void Ptrace::startTracing() {

    while (true) {

        int   status;
        pid_t waited_pid = waitpid(-1,      // wait for any child
                                   &status,
                                   __WALL); // wait for all children (including 'clone'd)
        if (waited_pid < 0) {
            assert(errno == ECHILD); // TODO: there are more cases (EINTER for example)
            break;
        } // else, one of the descendants changed state

        logger_ << "Process #" << waited_pid << " ";

        auto insert_result = process_list_.emplace(waited_pid); // TODO: 'find()' and then?
        if (insert_result.second) {
            // insertion took place, newborn process
            continue;
        }
        const auto& tracee = *insert_result.first;

        if (WIFEXITED(status)) {
            int retval = WEXITSTATUS(status);

            logger_ << "exited normally with value " << retval << Logger::endl;
            process_list_.erase(insert_result.first);
            event_callbacks_.onExit(waited_pid, retval);
            continue;

        } else if (WIFSIGNALED(status)) {
            int signal_num = WTERMSIG(status);
            char* signal_name = strsignal(signal_num);

            logger_ << "terminated by \"" << signal_name << "\" (#" << signal_num << ")"
                    << Logger::endl;
            process_list_.erase(insert_result.first);
            event_callbacks_.onTerminate(waited_pid, signal_num);
            continue;
        }

        assert(WIFSTOPPED(status));
        int signal_num       = WSTOPSIG(status);
        int signal_to_inject = 0;

        if (!tracee.flags.initialized_) {
            // after the newborn has stopped, we can now set the correct options
            auto flags = PTRACE_O_TRACESYSGOOD |
                         PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE;
            SAFE_SYSCALL(ptrace(PTRACE_SETOPTIONS, waited_pid, NULL, flags));

            tracee.flags.initialized_ = true;

        } else if (signal_num & PTRACE_O_TRACESYSGOOD_MASK) {
            assert(signal_num == (SIGTRAP | PTRACE_O_TRACESYSGOOD_MASK));

            Syscall syscall(static_cast<int>(SAFE_SYSCALL_BY_ERRNO(ptrace(PTRACE_PEEKUSER,
                                                                          waited_pid,
                                                                          REG_OFFSET(orig_rax)))));

            toggle(tracee.flags.in_syscall_);

            logger_ << "syscalled with \"" << syscall << "\"; "
                    << (tracee.flags.in_syscall_ ? "Enter" : "Exit") << Logger::endl;

            if (tracee.flags.in_syscall_) {
                event_callbacks_.onSyscallEnter(waited_pid, syscall);
            } else {
                event_callbacks_.onSyscallExit(waited_pid, syscall);
            }

        } else if (signal_num == SIGTRAP) {
            const auto event = status >> 16; // status>>8 == (SIGTRAP | PTRACE_EVENT_foo << 8)
            logger_ << "trapped. Event: " << event << Logger::endl;

            switch (event) {

                case PTRACE_EVENT_FORK:
                case PTRACE_EVENT_VFORK:
                case PTRACE_EVENT_CLONE:
                    break;
                case PTRACE_EVENT_EXEC:
                    throw std::logic_error("case not yet implemented"); // TODO
                default:
                    break; // TODO
            }

        } else {
            char* signal_name = strsignal(signal_num);

            logger_ << "signalled with \"" << signal_name << "\" (#" << signal_num << ")" << Logger::endl;
            event_callbacks_.onSignal(waited_pid, signal_num);

            signal_to_inject = signal_num;
        }

        SAFE_SYSCALL(ptrace(PTRACE_SYSCALL, waited_pid, NULL, signal_to_inject));
    }
}

void Ptrace::pokeSyscall(pid_t pid, Syscall syscall_to_run) {
    auto it = process_list_.find(TracedProcess(pid));// TODO is there a way not to create TracedProcess?
    if (it == process_list_.end()) {
        // TODO: throw? ignore?
    }

    assert(it->flags.in_syscall_);
    SAFE_SYSCALL(ptrace(PTRACE_POKEUSER, pid,
                        offsetof(struct user, regs.orig_rax), syscall_to_run.getSyscallNum()));
}