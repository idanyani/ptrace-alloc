//
// Created by mac on 3/8/19.
//

#ifndef PTRACE_ALLOC_TRACEE_SERVER_H
#define PTRACE_ALLOC_TRACEE_SERVER_H

#include <string>
#include <iostream>
/*
 * User should extend TraceeServer class
 */
 class TraceeServer{
  public:
     TraceeServer() : test_member_(0) {}
     enum Command { ALLOCATE_MEMORY, READ_FROM_FIFO, ASSERT_TEST_MEMBER, DEASSERT_TETS_MEMBER /*...*/ };
     void serveRequest(int fd);

     void assertTestMember();
     void deassertTestMember();

   private:
     int test_member_;
};

#endif //PTRACE_ALLOC_TRACEE_SERVER_H
