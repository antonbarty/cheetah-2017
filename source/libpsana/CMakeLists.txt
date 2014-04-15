
find_package(PSANA REQUIRED)

file(GLOB sources "${ANA_RELEASE}/psana/src/*.cpp")
list(REMOVE_ITEM sources "${ANA_RELEASE}/psana/src/EventLoop.cpp")
list(APPEND sources "src/EventLoop.cpp")
# Add source files here
include_directories("include")
INCLUDE_DIRECTORIES(${PSANA_INCLUDES})

add_definitions(-DCHEETAH_SIT_DATA="${ANA_SIT_DATA}")

add_library(libpsana SHARED ${sources})

target_link_libraries(libpsana libXtcInput libPSShmemInput ${PSANA_LIBRARIES})
SET_TARGET_PROPERTIES(libpsana PROPERTIES OUTPUT_NAME psana)

set_target_properties(
 libpsana
 PROPERTIES SOVERSION 1
 VERSION 1
)

install(TARGETS libpsana
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
