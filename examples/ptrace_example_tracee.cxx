#include <unistd.h>
#include <iostream>

int main() {
	pid_t pid = fork();
    std::cout << "TRACEE: got from fork:" << pid << std::endl;
    return pid;
}
