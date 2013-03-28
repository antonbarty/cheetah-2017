# - Find SPIMAGE library
# Find the native SPIMAGE includes and library
# This module defines
#  SPIMAGE_INCLUDE_DIR, where to find hdf5.h, etc.
#  SPIMAGE_LIBRARIES, libraries to link against to use SPIMAGE.
#  SPIMAGE_FOUND, If false, do not try to use SPIMAGE.
# also defined, but not for general use are
#  SPIMAGE_LIBRARY, where to find the SPIMAGE library.

find_file(ana_current_dir ana-current /opt/psana/g/psdm/portable/sw/releases/)

#find_library(ErrSvc_LIBRARY ErrSvc /reg/g/psdm/sw/releases/ana-current/arch/x86_64-rhel5-gcc41-opt/lib/ /opt/psana/g/psdm/portable/sw/releases/ana-current/arch/x86_64-rhel6-gcc44-opt/lib/)
