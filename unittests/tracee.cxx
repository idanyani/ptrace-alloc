
#include <unistd.h>
#include <csignal>
#include <iostream>
#include <sys/mman.h>

static const int command_fd = 3;
static const int result_fd  = 6;

int test0() {
    close(0);
    return 0;
}

int test1() {
    const auto res = (pid_t)(intptr_t) mmap(nullptr,
                                            4,
                                            PROT_READ | PROT_WRITE,
                                            MAP_PRIVATE | MAP_ANONYMOUS,
                                            -1,
                                            0);

    return write(result_fd, &res, sizeof(res)) != sizeof(res);
}

int main() {

    if (kill(getpid(), 0) != 0)  // this for caching main's start
        return -1;

    close(4);
    close(5);

    char command;
    if (read(command_fd, &command, sizeof(command)) < 1)
        return -1;

    switch (command) {
        case 0:
            return test0();
        case 1:
            return test1();
        default:
            return -1;
    }
}
