
#include <stdexcept>
#include "syscall.h"

#include "syscallents.h"

Syscall::operator std::string() const {
    if (num_ < 0) throw std::out_of_range("syscall number out of range");
    return std::string(syscalls[num_]);
}
