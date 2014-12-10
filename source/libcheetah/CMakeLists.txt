
find_package(HDF5 REQUIRED)
find_package(PythonLibs REQUIRED)
# We don't want to scare people with this
mark_as_advanced(HDF5_DIR)
mark_as_advanced(CLEAR HDF5_C_INCLUDE_DIR)
mark_as_advanced(CLEAR HDF5_hdf5_LIBRARY_RELEASE)
mark_as_advanced(CLEAR HDF5_hdf5_LIBRARY_DEBUG)


# Add source files here
LIST(APPEND sources "src/assemble2DImage.cpp" "src/backgroundCorrection.cpp")
LIST(APPEND sources "src/data2d.cpp" "src/detectorCorrection.cpp")
LIST(APPEND sources "src/frameBuffer.cpp")
LIST(APPEND sources "src/pixelmask.cpp")
LIST(APPEND sources "src/dataVersion.cpp" "src/detectorObject.cpp")
LIST(APPEND sources "src/hitfinders.cpp")	    
LIST(APPEND sources "src/downsample.cpp" "src/integratePattern.cpp")
LIST(APPEND sources "src/event.cpp" "src/libcheetah.cpp")
LIST(APPEND sources "src/median.cpp")
LIST(APPEND sources "src/powder.cpp" "src/peakfinders.cpp")
LIST(APPEND sources "src/radialAverage.cpp" "src/saveCXI.cpp")
LIST(APPEND sources "src/saveFrame.cpp" "src/global.cpp")
LIST(APPEND sources "src/spectrum.cpp" "src/worker.cpp")
LIST(APPEND sources "src/histogram.cpp" "src/processRateMonitor.cpp")
LIST(APPEND sources "src/tofDetector.cpp" "src/modularDetector.cpp")
LIST(APPEND sources "src/log.cpp" "src/peakDetect.cpp")
LIST(APPEND sources "src/gmd.cpp")

include_directories("include")
include_directories(${HDF5_INCLUDE_DIR})
include_directories(${PYTHON_INCLUDE_DIR})

add_library(cheetah SHARED ${sources})

target_link_libraries(cheetah ${HDF5_LIBRARIES} ${PYTHON_LIBRARIES})

set_target_properties(
 cheetah
 PROPERTIES SOVERSION 1
 VERSION 1
)

install(TARGETS cheetah
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
