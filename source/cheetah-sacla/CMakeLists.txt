
find_package(HDF5 REQUIRED)


LIST(APPEND sources "main-sacla-hdf5.cpp")
LIST(APPEND sources "sacla-hdf5-reader.cpp")
LIST(APPEND sources "sacla-hdf5-reader.h")

#add_definitions(-DCHEETAH_SIT_DATA="${ANA_SIT_DATA}")

include_directories("include")
include_directories(${CHEETAH_INCLUDES})
include_directories(${HDF5_INCLUDE_DIR})

#add_library(cheetah-sacla-1 SHARED ${sources})

add_executable(cheetah-sacla ${sources})

add_dependencies(cheetah-sacla cheetah)

target_link_libraries(cheetah-sacla ${CHEETAH_LIBRARY})

#set_target_properties(
# cheetah-sacla
# PROPERTIES SOVERSION 1
# VERSION 1
#)

install(TARGETS cheetah-sacla
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
