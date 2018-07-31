
#include <stdexcept>
#include "syscall.h"

#include "syscallents.h"


Syscall::Syscall(const std::string& syscall_name){

    for(int syscall_idx = 0; syscall_idx < MAX_SYSCALL_NUM; syscall_idx++){
        if(syscalls[syscall_idx] == syscall_name){
            num_ = syscall_idx;
            return;
        }
    }

    throw std::out_of_range("Invalid syscall name!");
}

Syscall::operator std::string() const {
    if (num_ < 0 || num_ >= MAX_SYSCALL_NUM) throw std::out_of_range("syscall number out of range");
    return std::string(syscalls[num_]);
}


