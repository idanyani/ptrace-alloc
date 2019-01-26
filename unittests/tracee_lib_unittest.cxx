//
// Created by mac on 1/25/19.
//

#include "Ptrace.h"
#include "PtraceTest.h"
#include "tracee_lib_event_callbacks.h"

#include <sys/types.h>

using ::testing::_;
using ::testing::AnyOf;
using ::testing::Contains;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Not;

TEST(PtraceLibTest, BasicTest){
    char* args[] = {const_cast<char*>("./tracee"), nullptr};
    TraceeLibEventCallbacks tracee_lib_event_callbacks;
    Ptrace ptrace(args[0], args, tracee_lib_event_callbacks);

    //ptrace.setLoggerVerbosity(Logger::Verbosity::ON);
    ptrace.startTracing();
}
