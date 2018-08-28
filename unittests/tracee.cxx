
#include <unistd.h>
#include <csignal>
#include <iostream>
#include <sys/mman.h>

static const int command_fd = 3;
static const int result_fd  = 6;

bool sendPid(pid_t pid) {
    return write(result_fd, &pid, sizeof(pid)) != sizeof(pid);
}

int test0() {
    return close(0);
}

int test1() {
    const auto pid = (pid_t)(intptr_t) mmap(nullptr,
                                            4,
                                            PROT_READ | PROT_WRITE,
                                            MAP_PRIVATE | MAP_ANONYMOUS,
                                            -1,
                                            0);

    return sendPid(pid);
}

int test2() {
    return kill(getpid(), SIGTERM);
}

int main() {
    close(4);
    close(5);

    char command;
    if (read(command_fd, &command, sizeof(command)) < 1)
        return -1;

    if (kill(getpid(), 0) != 0)  // this for caching test start
        return -1;

    switch (command) {
        case 0:
            return test0();
        case 1:
            return test1();
        case 2:
            return test2();
        default:
            return -1;
    }
}
