# List targets (if any) for this package
tgtnames := 

# List libraries (if any) for this package
libnames := client

# List source files for each library
unused_srcs    := VClientManager.cc clienttest.cc
libsrcs_client := $(filter-out $(unused_srcs), $(wildcard *.cc))

