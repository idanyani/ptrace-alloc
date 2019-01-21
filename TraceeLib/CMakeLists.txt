add_library(tracee_l SHARED tracee_lib.c)
#add_library(tracee_l_static STATIC tracee_lib.c)

install (TARGETS tracee_l DESTINATION bin)
install (FILES tracee_lib.h DESTINATION include)
