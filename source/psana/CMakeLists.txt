
find_package(PSANA REQUIRED)

file(GLOB sources "${ANA_RELEASE}/psana/app/*.cpp" "src/cheetah-psana.cpp")



INCLUDE_DIRECTORIES(${PSANA_INCLUDES})
add_definitions(-DCHEETAH_SIT_DATA="${ANA_SIT_DATA}")
include_directories("include")

ADD_EXECUTABLE(psana ${sources})

TARGET_LINK_LIBRARIES(psana ${PSANA_LIBRARIES} libpsana cheetah_ana_pkg)

INSTALL(TARGETS psana
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
