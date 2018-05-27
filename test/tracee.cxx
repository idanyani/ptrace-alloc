
#include <unistd.h>
#include <csignal>
#include <iostream>

int main() {

    getpid();
    close(0);

    return 0;
}
