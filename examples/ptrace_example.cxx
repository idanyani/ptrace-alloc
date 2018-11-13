#include <iostream>
#include <unistd.h>
#include "Ptrace.h"

using std::cout;
using std::endl;

struct SomeController {
    int  i;
    bool convertCloneToGetpidAndMakeItReturn3;

    SomeController() : i(657), convertCloneToGetpidAndMakeItReturn3(false) {}
};

class MyCallbacks : public Ptrace::EventCallbacks {
    SomeController& c;
  public:
    MyCallbacks(SomeController& c) : c(c) {}

    virtual void onExit(pid_t pid, int retval) {
        cout << "EXIT: PID: " << pid << "; retval:" << retval << endl;
        c.i = retval;
    }

    virtual void onStart(pid_t pid) {
        cout << "START: PID: " << pid << "C:" << c.i << endl;

    }

    virtual void onSyscallEnter(pid_t pid, Ptrace::SyscallEnterAction& action) {
        cout << "SYSENTER: PID: " << pid << "; retval:" << action.getSyscall() << endl;
        if (c.convertCloneToGetpidAndMakeItReturn3 && action.getSyscall() == Syscall("clone"))
            action.setSyscall(Syscall("getpid"));
    }

    virtual void onSyscallExit (pid_t pid, Ptrace::SyscallExitAction& action) {
        cout << "SYSEXIT : PID: " << pid << "; retval:" << action.getSyscall() << endl;
        if (c.convertCloneToGetpidAndMakeItReturn3 && action.getSyscall() == Syscall("getpid"))
            action.setReturnValue(3);
    }

};


int main() {
    char* args[] = {const_cast<char*>("ptrace_example_tracee"), nullptr};

    {
        SomeController c;
        MyCallbacks event_callbacks(c);

        Ptrace ptrace(args[0], args, event_callbacks);
        ptrace.startTracing();

        cout << "c.i:" << c.i << endl;
    }
    {
        SomeController c;
        MyCallbacks event_callbacks(c);

        c.convertCloneToGetpidAndMakeItReturn3 = true;

        Ptrace ptrace(args[0], args, event_callbacks);
        ptrace.startTracing();

        cout << "c.i:" << c.i << endl;
    }

    return 0;
}

