
#include <stdexcept>
#include "Syscall.h"

#include "Syscallents.h"


Syscall::Syscall(const std::string& syscall_name) {

    for (int syscall_idx = 0; syscall_idx < MAX_SYSCALL_NUM; syscall_idx++) {
        if (syscalls[syscall_idx] == syscall_name) {
            num_ = syscall_idx;
            return;
        }
    }

    throw std::out_of_range("Invalid syscall name!");
}

void Syscall::validate() const {
    if (num_ < 0 || num_ >= MAX_SYSCALL_NUM) throw std::out_of_range("syscall number out of range");
}

std::string Syscall::toString() const {
    return std::string(syscalls[getSyscallNum()]);
}

int Syscall::getSyscallNum() const {
    validate();
    return num_;
}



