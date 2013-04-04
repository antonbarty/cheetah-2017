
find_package(HDF5 REQUIRED)
# We don't want to scare people with this
mark_as_advanced(HDF5_DIR)
mark_as_advanced(CLEAR HDF5_C_INCLUDE_DIR)
mark_as_advanced(CLEAR HDF5_hdf5_LIBRARY_RELEASE)
mark_as_advanced(CLEAR HDF5_hdf5_LIBRARY_DEBUG)


file(GLOB sources "src/*.cpp")

include_directories("include")
include_directories(${HDF5_INCLUDE_DIR})

add_library(cheetah SHARED ${sources})

target_link_libraries(cheetah ${HDF5_LIBRARIES})

set_target_properties(
 cheetah
 PROPERTIES SOVERSION 1
 VERSION 1
)

install(TARGETS cheetah
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
