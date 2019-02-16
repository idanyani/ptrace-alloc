//
// Created by mac on 2/6/19.
//

#include "tracee_lib_test.h"
std::unique_ptr<Ptrace>
        initPtrace(char* args[], MockEventCallbacks& mock_event_callbacks) {
    std::unique_ptr<Ptrace> p_ptarce;

    p_ptarce.reset(new Ptrace(args[0], args, mock_event_callbacks));
    return p_ptarce;
}
