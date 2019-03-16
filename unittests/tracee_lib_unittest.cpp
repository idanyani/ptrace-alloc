#include <sys/types.h>
#include <tracee_server.h>
#include "tracee_lib_test.h"
#include "tracee_lib_fifo_test.h"

using ::testing::_;
using testing::Exactly;
using testing::AnyNumber;
using testing::AtLeast;


// Testing successful SIGUSR2 signal sending to the tracee (for now, verified via print)
TEST(TraceeLibTest, SendSiguser2Test){
    char* args[] = {const_cast<char*>("./tracee_basic"), nullptr};
    MockEventCallbacks mock_event_callbacks;

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
    MockEventCallbacks mock_event_callbacks;
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
                onExit(_, 0))
            .Times(Exactly(2));         // 2 tracees


    p_ptrace->startTracing();

}

TEST(TraceeLibTest, FifoBasicTest){                     // FIXME: make TraceeServer to accepr fifo messages
    SendMessageCallback sens_message_callback(0, std::string("mmap"));

    char* args[] = {const_cast<char*>("./tracee_fifo"), nullptr};

    std::unique_ptr<Ptrace> p_ptrace = initPtrace(args, sens_message_callback);


    EXPECT_CALL(sens_message_callback,
                onStart)
            .Times(Exactly(1));         // 2 tracees

    EXPECT_CALL(sens_message_callback,
                onSignal(_, SIGABRT))
            .Times(Exactly(0));

    EXPECT_CALL(sens_message_callback,
                onSignal(_, SIGSEGV))
            .Times(Exactly(0));

    EXPECT_CALL(sens_message_callback,
                onSyscallEnterT(_,_))
            .Times(AnyNumber());

    EXPECT_CALL(sens_message_callback,
                onSyscallExitT(_,_))
            .Times(AnyNumber());

    EXPECT_CALL(sens_message_callback,
                onSignal(_, SIGUSR2))
            .Times(Exactly(1));

    EXPECT_CALL(sens_message_callback,
                onSignal(_, SIGUSR1))
            .Times(Exactly(1));
    EXPECT_CALL(sens_message_callback,
                onExit(_,0))
            .Times(Exactly(1));

    p_ptrace->startTracing();

}


TEST(TraceeLibTest, SignalHandlersSurviveAfterExecveTest){          // FIXME make this taste use TraceeServer class
    char* args[] = {const_cast<char*>("./tracee_pingpong"), const_cast<char*>("2"), nullptr};

    SendMessageCallback send_message_callback(static_cast<int>(TraceeServer::Command::ASSERT_TEST_MEMBER), "mmap");

    std::unique_ptr<Ptrace> p_ptrace = initPtrace(args, send_message_callback);

    EXPECT_CALL(send_message_callback,
                onStart(_))
            .Times(Exactly(1));

    EXPECT_CALL(send_message_callback,
                onSignal(_, SIGABRT))
            .Times(Exactly(0));

    EXPECT_CALL(send_message_callback,
                onSignal(_, SIGSEGV))
            .Times(Exactly(0));

    EXPECT_CALL(send_message_callback,
                onSignal(_, SIGUSR1))
            .Times(AnyNumber());

    EXPECT_CALL(send_message_callback,
                onSignal(_, SIGUSR2))
            .Times(AnyNumber());

    EXPECT_CALL(send_message_callback,
                onExit(_, 0))
            .Times(Exactly(1));

    p_ptrace->startTracing();

}