
find_package(PSANA REQUIRED)

file(GLOB sources "${ANA_RELEASE}/XtcInput/src/*.cpp")
list(REMOVE_ITEM sources "${ANA_RELEASE}/XtcInput/src/DgramReader.cpp")
list(REMOVE_ITEM sources "${ANA_RELEASE}/XtcInput/src/StreamFileIterLive.cpp")
list(REMOVE_ITEM sources "${ANA_RELEASE}/XtcInput/src/RunFileIterLive.cpp")
list(APPEND sources "src/DgramReader.cpp")
list(APPEND sources "src/StreamFileIterLive.cpp")
list(APPEND sources "src/RunFileIterLive.cpp")
# Add source files here
include_directories("include")
INCLUDE_DIRECTORIES(${PSANA_INCLUDES})

add_definitions(-DCHEETAH_SIT_DATA="${ANA_SIT_DATA}")

add_library(libXtcInput SHARED ${sources})

target_link_libraries(libXtcInput ${PSANA_LIBRARIES})
SET_TARGET_PROPERTIES(libXtcInput PROPERTIES OUTPUT_NAME XtcInput)

set_target_properties(
 libXtcInput
 PROPERTIES SOVERSION 1
 VERSION 1
)

install(TARGETS libXtcInput
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
