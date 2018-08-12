// C++ headers
#include <iostream>
#include <vector>
#include <tuple>
#include <sstream>

// C headers
#include <unistd.h>
#include <cstring>

#include "ptrace.h"

using std::cout;
using std::endl;


int main(int argc, char* argv[]) {
    cout << "Tracer process id = " << getpid() << endl;

    // TODO: we can get the args via argv
    char* args[] = {const_cast<char*>("date"), nullptr};
    Ptrace ptrace("./child_mmap", args);

    while (true) {
        Ptrace::SyscallDirection direction;

        try {
            direction = ptrace.runUntilSyscallGate().second;

        } catch (std::exception& e) {
            cout << e.what() << endl;
            break;
        }

        cout << ptrace.getChildPid() << " " << (direction == Ptrace::SyscallDirection::ENTRY ?
                                           "enters kernel" : "exits kernel") << endl;
    }

    return 0;
}

