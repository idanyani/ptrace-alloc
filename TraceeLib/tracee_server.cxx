//
// Created by mac on 3/8/19.
//

#include "tracee_server.h"


void TraceeServer::serveRequest() {
    std::cout << "Serve request called" << std::endl;
}

//int TraceeServer::getFifoFd() const {
//    return fifo_fd_;
//}
//
//const std::string& TraceeServer::getFifoPath() const {
//    return fifo_path_;
//}
//
//void TraceeServer::setFifoFd(int fifo_fd) {
//    fifo_fd_ = fifo_fd;
//}
//
//void TraceeServer::setFifoPath(const std::string& fifo_path) {
//    fifo_path_ = std::string(fifo_path);
//}

pid_t TraceeServer::getPid() const {
    return tracee_pid_;
}

void TraceeServer::setPid(pid_t pid) {
    tracee_pid_ = pid;
}