
#include <sys/types.h>
#include "tracee_lib_test.h"

//int main(){
using ::testing::_;


// Testing successful SIGUSR2 signal sending to the tracee (for now, verified via print)
// FIXME: use google test features for verifying
TEST_F(TraceeLibTest, SendSiguser2Test){

    //std::cout << "SendSiguser2Test start " << getpid() << std::endl;
    EXPECT_CALL(mock_event_callbacks,
                onStart);

    EXPECT_CALL(mock_event_callbacks,
                onSignal(_, SIGUSR2));
    p_ptrace->startTracing();


    SUCCEED(); // Just want to make sure the test reached this point
}
