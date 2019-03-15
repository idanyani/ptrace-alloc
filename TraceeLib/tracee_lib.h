//
// Created by mac on 1/18/19.
//

#ifndef PTRACE_ALLOC_TRACEE_LIB_H
#define PTRACE_ALLOC_TRACEE_LIB_H

#include <string>
#include <memory>
#include "Ptrace.h"


class TraceeLibEventCallbacks : public Ptrace::EventCallbacks {

  public:
        //TraceeLibEventCallbacks(std::shared_ptr<Ptrace> ptrace_ptr) :
    //virtual int onSyscallExit (pid_t, Ptrace::SyscallExitAction&) override;

    TraceeLibEventCallbacks() = default;
    virtual ~TraceeLibEventCallbacks() = default;

    virtual void onStart    (pid_t) override ;

    virtual void onExit     (pid_t, int retval)     override ;
    virtual void onTerminate(pid_t, int signal_num) override ;
    virtual void onSignal   (pid_t, int signal_num) override ;

    virtual void onSyscallEnter(pid_t, Ptrace::SyscallEnterAction&) override ;
    virtual void onSyscallExit (pid_t, Ptrace::SyscallExitAction&)  override final;

    // Events callbacks
    virtual void onFork(pid_t)      override ;
    virtual void onVFork(pid_t)     override ;
    virtual void onVForkDone(pid_t) override ;
    virtual void onClone(pid_t)     override ;
    virtual void onExec(pid_t)    override ;

  protected:
    virtual int onSyscallExitInner(pid_t, Ptrace::SyscallExitAction&) { return 0; };

  private:
    //std::shared_ptr<Ptrace> ptrace_ptr_;

};


void setUserSignals();


#endif //PTRACE_ALLOC_TRACEE_LIB_H
