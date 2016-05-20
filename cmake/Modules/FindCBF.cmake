find_path(CBF_INCLUDE_DIR cbf.h)

find_library(CBF_LIBRARY NAMES cbf)

set(CBF_LIBRARIES ${LIBXML2_LIBRARY} )
set(CBF_INCLUDE_DIRS ${LIBXML2_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set CBF_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(CBF  DEFAULT_MSG
                                  CBF_LIBRARY CBF_INCLUDE_DIR)

