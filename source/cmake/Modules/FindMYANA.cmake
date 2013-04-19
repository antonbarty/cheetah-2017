find_file(MYANA_RELEASE release /reg/g/pcds/package/ana /davinci/pcds/package/ana)


set(MYANA_ARCH "x86_64-linux-opt" ACHE STRING "myana architecture to be used")

LIST(APPEND myana_libs acqdata
anadata
andordata
appdata
bld
camdata
compressdata
controldata
cspad2x2data
cspaddata
encoderdata
epics
evrdata
fccddata
fexampdata
flidata
gsc16aidata
indexdata
ipimbdata
lusidata
oceanopticsdata
opal1kdata
phasicsdata
pnccddata
princetondata
pulnixdata
quartzdata
timepixdata
usdusbdata
xampsdata
xtcdata
xtcrunset)

foreach(myana_lib IN LISTS myana_libs)
  # Clear variable first
  SET(MYANA_${myana_lib}_LIBRARY "MYANA_${myana_lib}_LIBRARY-NOTFOUND" CACHE INTERNAL "Internal" FORCE)
  find_library(MYANA_${myana_lib}_LIBRARY ${myana_lib} ${MYANA_RELEASE}/build/pdsdata/lib/${MYANA_ARCH})
  SET(MYANA_${myana_lib}_LIBRARY ${MYANA_${myana_lib}_LIBRARY} CACHE INTERNAL "Internal" FORCE)
#  mark_as_advanced(MYANA_${myana_lib}_LIBRARY)
  message(STATUS "Found ${myana_lib} in ${MYANA_${myana_lib}_LIBRARY}")
  LIST(APPEND MYANA_LIBRARIES ${MYANA_${myana_lib}_LIBRARY})
endforeach(myana_lib)

#clear var
SET(MYANA_INCLUDES)
LIST(APPEND MYANA_INCLUDES ${MYANA_RELEASE})
