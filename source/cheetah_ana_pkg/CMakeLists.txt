
find_package(PSANA REQUIRED)

find_package(HDF5 REQUIRED)


LIST(APPEND sources "src/cheetah_ana_mod.cpp" "src/event_copy.cpp")

add_definitions(-DCHEETAH_SIT_DATA="${ANA_SIT_DATA}")

include_directories("include")
#include_directories("../libpsana/include")
include_directories(${PSANA_INCLUDES} ${CHEETAH_INCLUDES})
include_directories(${HDF5_INCLUDE_DIR})

add_library(cheetah_ana_pkg SHARED ${sources})

add_dependencies(cheetah_ana_pkg cheetah)

target_link_libraries(cheetah_ana_pkg ${PSANA_LIBRARIES} ${CHEETAH_LIBRARY})

set_target_properties(
 cheetah_ana_pkg
 PROPERTIES SOVERSION 1
 VERSION 1
)

install(TARGETS cheetah_ana_pkg
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
