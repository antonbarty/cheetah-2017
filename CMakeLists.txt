cmake_minimum_required(VERSION 2.6)

PROJECT (cheetah)

INCLUDE(CTest)
ENABLE_TESTING()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
# Provide the sha1 of the current git commit as a #define
add_definitions("-DGIT_SHA1=\"${GIT_SHA1}\"")


SET(ENABLE_RUNPATH FALSE CACHE BOOL "Try to use DT_RUNPATH on binaries. Unlikely to work.")
MARK_AS_ADVANCED(ENABLE_RUNPATH)
IF(CMAKE_COMPILER_IS_GNUCC)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra -Wall -std=c99")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wall")
	if(ENABLE_RUNPATH)
		# Use RUNPATH instead of RPATH
		SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
		SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--enable-new-dtags")
	endif(ENABLE_RUNPATH)
ENDIF(CMAKE_COMPILER_IS_GNUCC)

SET(RPATH_ON_INSTALLED_BINS TRUE CACHE BOOL "Keep RPATH on installed binaries")
mark_as_advanced(RPATH_ON_INSTALLED_BINS)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH ${RPATH_ON_INSTALLED_BINS})

# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")

OPTION(BUILD_CHEETAH_ANA_MOD "If ON build cheetah_ana_mod. Otherwise skip it." OFF )
OPTION(BUILD_PSANA "If ON build psana. Otherwise skip it." OFF )
OPTION(BUILD_CHEETAH_MYANA "If ON build cheetah_myana. Otherwise skip it." OFF )
OPTION(BUILD_CHEETAH_SACLA "If ON build cheetah-sacla. Otherwise skip it." OFF )

SET(CHEETAH_INCLUDES ${CMAKE_SOURCE_DIR}/source/libcheetah/include CACHE PATH "libcheetah include directory")
MARK_AS_ADVANCED(CHEETAH_INCLUDES)

SET(CHEETAH_LIBRARY ${CMAKE_BINARY_DIR}/source/libcheetah/libcheetah.so CACHE FILEPATH "libcheetah to link against")
MARK_AS_ADVANCED(CHEETAH_LIBRARY)


ADD_SUBDIRECTORY(source)

IF(BUILD_TESTING)
ADD_SUBDIRECTORY(tests)
ENDIF(BUILD_TESTING)
