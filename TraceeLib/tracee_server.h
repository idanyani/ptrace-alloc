//
// Created by mac on 3/8/19.
//

#ifndef PTRACE_ALLOC_TRACEE_SERVER_H
#define PTRACE_ALLOC_TRACEE_SERVER_H

#include <string>
#include <iostream>
/*
 * User should extend TraceeServer class
 */
 class TraceeServer{
  public:
    enum class Command { ALLOCATE_MEMORY, READ_FROM_FIFO /*...*/ };
    //void serveRequest(Command command_to_execute, int command_argument); // FIXME: use serveCommand that accepts parameters
    void serveRequest();

    pid_t getPid() const;
    //int getFifoFd() const;
    //const std::string& getFifoPath() const;

    void setPid(pid_t pid);
    //void setFifoFd(int fifo_fd);
    //void setFifoPath(const std::string& fifo_path);

  private:
     pid_t tracee_pid_;
     //int fifo_fd_;
     //std::string fifo_path_;

};

#endif //PTRACE_ALLOC_TRACEE_SERVER_H