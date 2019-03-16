
#include <sys/types.h>
#include "tracee_lib_test.h"
#include "tracee_lib_fifo_test.h"

using ::testing::_;
using testing::Exactly;
using testing::AnyNumber;
using testing::AtLeast;


// Testing successful SIGUSR2 signal sending to the tracee (for now, verified via print)
TEST(TraceeLibTest, SendSiguser2Test){
    char* args[] = {const_cast<char*>("./tracee_basic"), nullptr};
    TraceeLibMockEventCallbacks mock_event_callbacks;

    std::unique_ptr<Ptrace> p_ptrace = initPtrace(args, mock_event_callbacks);


    EXPECT_CALL(mock_event_callbacks,
                onStart)
            .Times(Exactly(1));

    EXPECT_CALL(mock_event_callbacks,
                onSignal(_, SIGABRT))
            .Times(Exactly(0));

    EXPECT_CALL(mock_event_callbacks,
                onSignal(_, SIGSEGV))
            .Times(Exactly(0));

    EXPECT_CALL(mock_event_callbacks,
                onSignal(_, SIGUSR2)) // After tracing kill(get_pid(), 0) in tracee lib constructor,
            .Times(Exactly(2));         // tracer sends SIGUSR2 to the tracee. Since after execv, the constructor
                                        // is called, SIGUSR2 will be sent once again
    EXPECT_CALL(mock_event_callbacks,
                onExec(_)) // When calling execv, tracee gets SIGTRAP because of PTRACE_O_TRACEEXEC option
            .Times(Exactly(1));

    EXPECT_CALL(mock_event_callbacks,
                onSyscallEnterT(_,_))
            .Times(AnyNumber());
    EXPECT_CALL(mock_event_callbacks,
                onSyscallExitT(_,_))
            .Times(AnyNumber());

    EXPECT_CALL(mock_event_callbacks,
                onExit)
            .Times(Exactly(1));

    p_ptrace->startTracing();
}

TEST(TraceeLibTest, ForkTest){
    TraceeLibMockEventCallbacks mock_event_callbacks;
    char* args[] = {const_cast<char*>("./tracee_fork"), nullptr};

    std::unique_ptr<Ptrace> p_ptrace = initPtrace(args, mock_event_callbacks);

    EXPECT_CALL(mock_event_callbacks,
                onStart)
            .Times(Exactly(2));         // 2 tracees

    EXPECT_CALL(mock_event_callbacks,
                onSignal(_, SIGABRT))
            .Times(Exactly(0));

    EXPECT_CALL(mock_event_callbacks,
                onSignal(_, SIGSEGV))
            .Times(Exactly(0));

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
                onFork(_))
            .Times(Exactly(1));         // 1 fork

    EXPECT_CALL(mock_event_callbacks,
                onExec(_))
            .Times(Exactly(2));         // 2 execve's

    EXPECT_CALL(mock_event_callbacks,
                onSignal(_, SIGCHLD))
            .Times(AnyNumber());        // because of tracee's child process

    EXPECT_CALL(mock_event_callbacks,
                onExit)
            .Times(Exactly(2));         // 2 tracees

    p_ptrace->startTracing();

}

TEST(TraceeLibTest, FifoBasicTest){
    SendMessageCallback send_message_callbacks(std::string("getcwd"));

    char* args[] = {const_cast<char*>("./tracee_fifo"), nullptr};
    std::unique_ptr<Ptrace> p_ptrace = initPtrace(args, send_message_callbacks);



    EXPECT_CALL(send_message_callbacks,
                onStart)
            .Times(Exactly(1));         // 2 tracees

    EXPECT_CALL(send_message_callbacks,
                onSignal(_, SIGABRT))
            .Times(Exactly(0));

    EXPECT_CALL(send_message_callbacks,
                onSignal(_, SIGSEGV))
            .Times(Exactly(0));

    EXPECT_CALL(send_message_callbacks,
                onSyscallEnterT(_,_))
            .Times(AnyNumber());

    EXPECT_CALL(send_message_callbacks,
                onSyscallExitT(_,_))
            .Times(AnyNumber());

    EXPECT_CALL(send_message_callbacks,
                onSignal(_, SIGUSR2))
            .Times(Exactly(1));

    EXPECT_CALL(send_message_callbacks,
                onSignal(_, SIGUSR1))
            .Times(Exactly(1));
    EXPECT_CALL(send_message_callbacks,
                onExit(_,_))
            .Times(Exactly(1));

    p_ptrace->startTracing();


}


//TEST(TraceeLibTest, SignalHandlersSurviveAfterExecveTest){          // FIXME make this taste use TraceeServer class
//    char* args[] = {const_cast<char*>("./tracee_pingpong"), const_cast<char*>("2"), nullptr};
//    SendSignalOnMmapCallback mock_event_callbacks;
//
//    std::unique_ptr<Ptrace> p_ptrace = initPtrace(args, mock_event_callbacks);
//
//    EXPECT_CALL(mock_event_callbacks,
//                onSignal(_, SIGABRT))
//            .Times(Exactly(0));
//
//    EXPECT_CALL(mock_event_callbacks,
//                onSignal(_, SIGSEGV))
//            .Times(Exactly(0));
//
//    EXPECT_CALL(mock_event_callbacks,
//                onSignal(_, SIGUSR1))
//            .Times(AnyNumber());
//
//    EXPECT_CALL(mock_event_callbacks,
//                onSignal(_, SIGUSR2))
//            .Times(AnyNumber());
//
//    EXPECT_CALL(mock_event_callbacks,
//                onSyscallEnterT(_,_))
//            .Times(AnyNumber());
//
//    EXPECT_CALL(mock_event_callbacks,
//                onSyscallExitT(_,_))
//            .Times(AnyNumber());
//
//    p_ptrace->startTracing();
//
//}




