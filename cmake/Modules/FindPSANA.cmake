
find_file(ANA_CURRENT_ROOT ana-current /opt/psana/g/psdm/portable/sw/releases/)

file(READ /etc/redhat-release redhat_release)
string(REGEX MATCH " 6\\." rhel6 ${redhat_release})
string(REGEX MATCH " 5\\." rhel5 ${redhat_release})
if(rhel6)
 set(arch "rhel6")
endif(rhel6)
if(rhel5)
 set(arch "rhel5")
endif(rhel5)

set(ANA_ARCH "x86_64-${arch}-gcc44-opt" CACHE STRING "ana architecture to be used")

find_library(ANA_ErrSvc_LIBRARY ErrSvc ${ANA_CURRENT_ROOT}/arch/${ANA_ARCH}/lib/)

mark_as_advanced(ANA_ErrSvc_LIBRARY)
