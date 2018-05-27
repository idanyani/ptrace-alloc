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
    char* args[] = {const_cast<char*>("date"), NULL};
    Ptrace ptrace("./child_mmap", args);

    while (1) {
        Ptrace::SyscallDirection direction;

        try {
            direction = ptrace.runUntilSyscallGate();

        } catch (std::exception& e) {
            cout << e.what() << endl;
            break;
        }

        cout << ptrace.getPid() << " " << (direction == Ptrace::SyscallDirection::ENTRY ?
                                           "enters kernel" : "exits kernel") << endl;
    }

    return 0;
}

