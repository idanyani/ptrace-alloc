
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
    MockEventCallbacks mock_event_callbacks;

    std::unique_ptr<Ptrace> p_ptrace = initPtrace(args, mock_event_callbacks);

    EXPECT_CALL(mock_event_callbacks,
                onStart)
            .Times(Exactly(1));

    EXPECT_CALL(mock_event_callbacks,
                onSignal(_, SIGUSR2)) // After tracing kill(get_pid(), 0) in tracee lib constructor,
            .Times(Exactly(2));         // tracer sends SIGUSR2 to the tracee. Since after execv, the constructor
                                        // is called, SIGUS@ will be sent once again
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
//    MockEventCallbacks mock_event_callbacks;
    SendMessageCallback fifoCallbacks(std::string("getcwd"));

    char* args[] = {const_cast<char*>("./tracee_fifo"), const_cast<char*>("0"), nullptr};
    Ptrace ptrace(args[0], args, fifoCallbacks);

    ptrace.setLoggerVerbosity(Logger::Verbosity::OFF);

    EXPECT_CALL(fifoCallbacks,
                onStart)
            .Times(Exactly(1));         // 2 tracees

    EXPECT_CALL(fifoCallbacks,
                onSyscallEnterT(_,_))
            .Times(AnyNumber());

    EXPECT_CALL(fifoCallbacks,
                onSyscallExitT(_,_))
            .Times(AnyNumber());

    EXPECT_CALL(fifoCallbacks,
                onSignal(_, SIGUSR2))
            .Times(Exactly(1));

    EXPECT_CALL(fifoCallbacks,
                onSignal(_, SIGUSR1))
            .Times(Exactly(1));
    EXPECT_CALL(fifoCallbacks,
                onExit(_,_))
            .Times(Exactly(1));
/*
    EXPECT_CALL(fifoCallbacks,
                onSyscallExitT(_, Syscall("read")))
            .Times(AtLeast(1));
  */
    ptrace.startTracing();

    SUCCEED();
}

TEST(TraceeLibTest, SendMessageOnMmapTest){
//    MockEventCallbacks mock_event_callbacks;
    SendMessageCallback fifoCallbacks(std::string("mmap"));

    char* args[] = {const_cast<char*>("./tracee_fifo"), const_cast<char*>("0"), nullptr};
    Ptrace ptrace(args[0], args, fifoCallbacks);

    EXPECT_CALL(fifoCallbacks,
                onStart)
            .Times(Exactly(1));         // 2 tracees

    EXPECT_CALL(fifoCallbacks,
                onSyscallEnterT(_,_))
            .Times(AnyNumber());

    EXPECT_CALL(fifoCallbacks,
                onSyscallExitT(_,_))
            .Times(AnyNumber());

    EXPECT_CALL(fifoCallbacks,
                onSignal(_, SIGUSR2))
            .Times(Exactly(1));

    EXPECT_CALL(fifoCallbacks,
                onSignal(_, SIGUSR1))
            .Times(Exactly(1));
    EXPECT_CALL(fifoCallbacks,
                onExit(_,_))
            .Times(Exactly(1));

    ptrace.setLoggerVerbosity(Logger::Verbosity::OFF);

    ptrace.startTracing();

    SUCCEED();
}

TEST(TraceeLibTest, SendMessageOnMmapAndExecveTest){
//    MockEventCallbacks mock_event_callbacks;
    SendMessageCallback fifoCallbacks(std::string("mmap"));

    char* args[] = {const_cast<char*>("./tracee_fifo"), const_cast<char*>("1"), nullptr};
    Ptrace ptrace(args[0], args, fifoCallbacks);


    EXPECT_CALL(fifoCallbacks,
                onStart)
            .Times(Exactly(1));         // 2 tracees

    EXPECT_CALL(fifoCallbacks,
                onSyscallEnterT(_,_))
            .Times(AnyNumber());

    EXPECT_CALL(fifoCallbacks,
                onSyscallExitT(_,_))
            .Times(AnyNumber());

    EXPECT_CALL(fifoCallbacks,
                onSignal(_, SIGUSR2))
            .Times(Exactly(3));                 // starting + 2 execve's = 3 signals
    EXPECT_CALL(fifoCallbacks, onExec(_))
            .Times(Exactly(2));                 // exec to tracee_basic and then to /bin/date

    EXPECT_CALL(fifoCallbacks,
                onSignal(_, SIGUSR1))
            .Times(AtLeast(2));                 // we called mmap twice, any additional mmap
                                                // calls will cause sending SIGUSR1
    EXPECT_CALL(fifoCallbacks,
                onExit(_,_))
            .Times(Exactly(1));

//    EXPECT_CALL(fifoCallbacks,
//                onSyscallExitT(_, SyscallEq<Ptrace::SyscallExitAction>(Syscall("mmap"))))
//            .Times(AtLeast(2));

    //ptrace.setLoggerVerbosity(Logger::Verbosity::OFF);

    ptrace.startTracing();

    SUCCEED();
}

TEST(TraceeLibTest, ExecSanityTest){ // FIXME: delete?
    char* args[] = {const_cast<char*>("./tracee_basic"), nullptr};
    MockEventCallbacks mock_event_callbacks;

    std::unique_ptr<Ptrace> p_ptrace = initPtrace(args, mock_event_callbacks);

    EXPECT_CALL(mock_event_callbacks,
                onSyscallEnterT(_,_))
            .Times(AnyNumber());

    EXPECT_CALL(mock_event_callbacks,
                onSyscallExitT(_,_))
            .Times(AnyNumber());

    p_ptrace->startTracing();

    SUCCEED();
}

TEST(TraceeLibTest, SignalHandlersSurviveAfterExecveTest){
    char* args[] = {const_cast<char*>("./tracee_pingpong"), const_cast<char*>("2"), nullptr};
    SendSignalOnMmapCallback mock_event_callbacks;

    std::unique_ptr<Ptrace> p_ptrace = initPtrace(args, mock_event_callbacks);
    p_ptrace->startTracing();

    SUCCEED();
}




