add_library(tracee_l SHARED tracee_lib.cxx tracee_server.cxx tracee_lib_defines.h)

install (TARGETS tracee_l DESTINATION bin)
install (FILES tracee_lib.h DESTINATION include)
