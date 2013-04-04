find_file(MYANA_RELEASE release /reg/g/pcds/package/ana /opt/pcds/package/ana)


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
  message(STATUS "locating ${myana_lib}")
  find_library(MYANA_${myana_lib}_LIBRARY ${myana_lib} ${MYANA_RELEASE}/build/pdsdata/lib/${MYANA_ARCH})
mark_as_advanced(MYANA_${myana_lib}_LIBRARY)
LIST(APPEND MYANA_LIBRARIES ${MYANA_${myana_lib}_LIBRARY})
endforeach(myana_lib)

LIST(APPEND MYANA_INCLUDES ${MYANA_RELEASE})