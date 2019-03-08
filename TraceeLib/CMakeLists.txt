add_library(tracee_l SHARED tracee_lib.cxx tracee_server.cxx)

install (TARGETS tracee_l DESTINATION bin)
install (FILES tracee_lib.h DESTINATION include)
