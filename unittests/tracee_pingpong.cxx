//
// Created by mac on 3/6/19.
//

#include <cassert>
#include <string>
#include <unistd.h>
#include <cstring>
#include <sys/mman.h>

#include <signal.h>
#include <iostream>
#include <tracee_server.h>

extern TraceeServer tracee_server;

int main(int argc, char** argv){
    assert(argc > 1);
    std::string arg(argv[1]);
    int num = std::stoi(arg), address;
    // FIXME: set signal handler


    address = (intptr_t) mmap(NULL,
                              4,
                              PROT_READ | PROT_WRITE,
                              MAP_PRIVATE | MAP_ANONYMOUS,
                              -1,
                              0);

    assert(tracee_server.getTestMember() == 1);

    if(address) ;

    if(num > 0){
        char buff[64];
        std::string new_arg(std::to_string(num - 1));
        strcpy(buff, new_arg.c_str());

        char* new_args[] = {argv[0], buff, 0};
        execv("./tracee_pingpong", new_args);
    }

    return 0;
}