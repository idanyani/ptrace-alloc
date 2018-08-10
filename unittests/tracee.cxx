
#include <unistd.h>
#include <csignal>
#include <iostream>

int main() {

    if (kill(getpid(), 0) != 0) return -1;
    close(0);

    return 0;
}
