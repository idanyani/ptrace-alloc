//
// Created by mac on 3/8/19.
//

#include <unistd.h>

#include "tracee_server.h"
#include "tracee_lib_defines.h"


void TraceeServer::serveRequest(int fd) {

    char command_type_str[64];
    TRACEE_SAFE_SYSCALL(read(fd, command_type_str, 64));

    std::string command_type_wrap(command_type_str);

    Command command = static_cast<Command>(stoi(command_type_wrap));

    switch(command){
        case Command::ALLOCATE_MEMORY     :   ;                     break;
        case Command::READ_FROM_FIFO      :   ;                     break;
        case Command::ASSERT_TEST_MEMBER  : assertTestMember();     break;
        case Command::DEASSERT_TETS_MEMBER: deassertTestMember();   break;
    }

}

void TraceeServer::assertTestMember() {
    test_member_ = 1;
}

void TraceeServer::deassertTestMember() {
    test_member_ = 0;
}

int TraceeServer::getTestMember() const {
    return test_member_;
}

