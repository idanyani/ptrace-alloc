// C++ headers
#include <iostream>
#include <vector>
#include <tuple>
#include <sstream>

// C headers
#include <unistd.h>
#include <cstring>

#include "Ptrace.h"

using std::cout;
using std::endl;


int main(int argc, char* argv[]) {
    cout << "Tracer process id = " << getpid() << endl;

    // TODO: we can get the args via argv
    char* args[] = {const_cast<char*>("date"), nullptr};
    Ptrace::EventCallbacks event_callbacks;
    Ptrace ptrace("./child_mmap", args, event_callbacks);

    return 0;
}

