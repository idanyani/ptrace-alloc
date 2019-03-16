//
// Created by mac on 2/6/19.
//

#ifndef PTRACE_ALLOC_TRACEE_LIB_TEST_H
#define PTRACE_ALLOC_TRACEE_LIB_TEST_H

#include "Ptrace.h"
#include "PtraceTest.h"
#include <memory>        // unique_ptr
#include <unordered_map>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

std::unique_ptr<Ptrace> initPtrace(char* args[], MockEventCallbacks& mock_event_callbacks);


#endif //PTRACE_ALLOC_TRACEE_LIB_TEST_H