
find_package(MYANA REQUIRED)

file(GLOB sources "src/*.cpp")
list(APPEND sources "src/cheetah-myana.cpp" "src/myana/main.cc")

IF(CMAKE_COMPILER_IS_GNUCC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

include_directories("include")
include_directories(${PSANA_INCLUDES} ${CHEETAH_INCLUDES})
include_directories(${MYANA_INCLUDES})

add_executable(cheetah_myana ${sources})

add_dependencies(cheetah_myana cheetah)

target_link_libraries(cheetah_myana ${CHEETAH_LIBRARY} ${MYANA_LIBRARIES})

install(TARGETS cheetah_myana
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
