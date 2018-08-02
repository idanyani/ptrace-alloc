#ifndef PTRACE_ALLOC_SYSCALL_H
#define PTRACE_ALLOC_SYSCALL_H

#include <string>

class Syscall {
  public:
    explicit Syscall(int num = -1) : num_(num) {}

    explicit Syscall(const std::string& syscall_name);

    explicit operator std::string() const;

    bool operator==(const Syscall& other) const {
        return num_ == other.num_;
    }

    bool operator!=(const Syscall& other) const {
        return num_ != other.num_;
    }

    int getSyscallNum() const  { return num_; }

  private:
    int num_;
};


#endif //PTRACE_ALLOC_SYSCALL_H
