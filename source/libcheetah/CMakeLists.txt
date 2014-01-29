
FIND_PACKAGE(HDF5 REQUIRED)
# we don't want to scare people with this
MARK_AS_ADVANCED(HDF5_DIR)
MARK_AS_ADVANCED(CLEAR HDF5_C_INCLUDE_DIR)
MARK_AS_ADVANCED(CLEAR HDF5_HDF5_LIBRARY_RELEASE)
MARK_AS_ADVANCED(CLEAR HDF5_HDF5_LIBRARY_DEBUG)


# add source files here
LIST(APPEND SOURCES "src/assemble2DImage.cpp" "src/backgroundCorrection.cpp")
LIST(APPEND SOURCES "src/data2d.cpp" "src/detectorCorrection.cpp")
LIST(APPEND SOURCES "src/detectorObject.cpp" "src/hitfinders.cpp")
LIST(APPEND SOURCES "src/libcheetah.cpp" "src/median.cpp")
LIST(APPEND SOURCES "src/powder.cpp" "src/peakfinders.cpp")
LIST(APPEND SOURCES "src/radialAverage.cpp" "src/saveCXI.cpp")
LIST(APPEND SOURCES "src/saveFrame.cpp" "src/setup.cpp")
LIST(APPEND SOURCES "src/spectrum.cpp" "src/worker.cpp")
LIST(APPEND SOURCES "src/histogram.cpp" )

INCLUDE_DIRECTORIES("include")
INCLUDE_DIRECTORIES(${HDF5_INCLUDE_DIR})

ADD_LIBRARY(cheetah SHARED ${SOURCES})

TARGET_LINK_LIBRARIES(cheetah ${HDF5_LIBRARIES})

SET_TARGET_PROPERTIES(
 cheetah
 PROPERTIES SOVERSION 1
 VERSION 1
)

INSTALL(TARGETS cheetah
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
