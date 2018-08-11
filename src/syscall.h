#ifndef PTRACE_ALLOC_SYSCALL_H
#define PTRACE_ALLOC_SYSCALL_H

#include <string>

class Syscall {
  public:
    explicit Syscall(int num = -1) : num_(num) {}

    explicit Syscall(const std::string& syscall_name);


    bool operator==(const Syscall& other) const {
        return num_ == other.num_;
    }

    bool operator!=(const Syscall& other) const {
        return num_ != other.num_;
    }

    int getSyscallNum() const;

    std::string toString() const;

  private:
    int num_;
    void validate() const;
};

static inline
std::ostream& operator<<(std::ostream& os, const Syscall& syscall) {
    os << syscall.toString();
    return os;
}

#endif //PTRACE_ALLOC_SYSCALL_H
