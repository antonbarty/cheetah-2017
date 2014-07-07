
find_package(PSANA REQUIRED)

file(GLOB sources "${ANA_RELEASE}/PSShmemInput/src/*.cpp")
# Add source files here
include_directories("include")
include_directories("../libXtcInput/include")
INCLUDE_DIRECTORIES(${PSANA_INCLUDES})

add_definitions(-DCHEETAH_SIT_DATA="${ANA_SIT_DATA}")

add_library(libPSShmemInput SHARED ${sources})

target_link_libraries(libPSShmemInput libXtcInput ${PSANA_LIBRARIES})
SET_TARGET_PROPERTIES(libPSShmemInput PROPERTIES OUTPUT_NAME PSShmemInput)

set_target_properties(
 libPSShmemInput
 PROPERTIES SOVERSION 1
 VERSION 1
)

install(TARGETS libPSShmemInput
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
