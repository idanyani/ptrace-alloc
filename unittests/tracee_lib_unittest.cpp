
#include <sys/types.h>
#include "tracee_lib_test.h"

using ::testing::_;
using testing::Exactly;
using testing::AnyNumber;


// Testing successful SIGUSR2 signal sending to the tracee (for now, verified via print)
TEST(TraceeLibTest, SendSiguser2Test){
    char* args[] = {const_cast<char*>("./tracee_basic"), nullptr};
    MockEventCallbacks mock_event_callbacks;

    std::unique_ptr<Ptrace> p_ptrace = initPtrace(args, mock_event_callbacks);

    EXPECT_CALL(mock_event_callbacks,
                onStart);

    EXPECT_CALL(mock_event_callbacks,
                onSignal(_, SIGUSR2));

    EXPECT_CALL(mock_event_callbacks,
                onExit);
    p_ptrace->startTracing();

}

TEST(TraceeLibTest, ForkTest){
    MockEventCallbacks mock_event_callbacks;
    char* args[] = {const_cast<char*>("./tracee_fork"), nullptr};

    std::unique_ptr<Ptrace> p_ptrace = initPtrace(args, mock_event_callbacks);

    EXPECT_CALL(mock_event_callbacks,
                onStart)
            .Times(Exactly(2));         // 2 tracees

    EXPECT_CALL(mock_event_callbacks,
                onSyscallEnterT(_,_))
            .Times(AnyNumber());
    EXPECT_CALL(mock_event_callbacks,
                onSyscallExitT(_,_))
            .Times(AnyNumber());

    EXPECT_CALL(mock_event_callbacks,
                onSignal(_, SIGUSR2))
            .Times(Exactly(3));         // 1 tracee starts from the beginning, another does execve twice
    EXPECT_CALL(mock_event_callbacks,
                onSignal(_, SIGTRAP))
            .Times(Exactly(3));         // 1 fork and 2 execve's

    EXPECT_CALL(mock_event_callbacks,
                onExit)
            .Times(Exactly(2));         // 2 tracees

    p_ptrace->startTracing();

}

/*
TEST(TraceeLibTest, ForkStressTest){
    int
    MockEventCallbacks mock_event_callbacks;
    char* args[] = {const_cast<char*>("./tracee_fork"), nullptr};

    std::unique_ptr<Ptrace> p_ptrace = initPtrace(args, mock_event_callbacks);


}
*/
// FIXME: add FIFO test
