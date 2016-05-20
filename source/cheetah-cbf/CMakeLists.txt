
find_package(HDF5 REQUIRED)
find_package(CBF REQUIRED)


LIST(APPEND sources "main-cbf.cpp")

include_directories(${CHEETAH_INCLUDES} ${HDF5_INCLUDE_DIR} ${CBF_INCLUDE_DIR})

add_executable(cheetah-cbf ${sources})

add_dependencies(cheetah-cbf cheetah)

target_link_libraries(cheetah-cbf ${CHEETAH_LIBRARY} ${HDF5_LIBRARIES} ${CBF_LIBRARY} )

install(TARGETS cheetah-cbf
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
