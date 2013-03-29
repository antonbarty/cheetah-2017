
find_file(ANA_CURRENT_ROOT ana-current /opt/psana/g/psdm/portable/sw/releases/)

file(READ /etc/redhat-release redhat_release)
string(REGEX MATCH " 6\\." rhel6 ${redhat_release})
string(REGEX MATCH " 5\\." rhel5 ${redhat_release})
if(rhel6)
 set(arch "rhel6")
endif(rhel6)
if(rhel5)
 set(arch "rhel5")
endif(rhel5)

set(ANA_ARCH "x86_64-${arch}-gcc44-opt" CACHE STRING "ana architecture to be used")

LIST(APPEND ana_libs ErrSvc PSTime rt PSEvt AppUtils acqdata andordata bld camdata compressdata controldata
  cspad2x2data cspaddata encoderdata epics evrdata fccddata fexampdata flidata gsc16aidata indexdata
  ipimbdata lusidata oceanopticsdata opal1kdata orcadata phasicsdata pnccddata princetondata pulnixdata
  quartzdata timepixdata usdusbdata xampsdata xtcdata ConfigSvc MsgLogger PSEnv RootHistoManager PSHist
  ExpNameDb psddl_psana psana Core Cint RIO Net Hist Graf Graf3d Gpad Tree Rint Postscript Matrix Physics MathCore Thread
  m dl)

foreach(ana_lib IN LISTS ana_libs)
	message(STATUS "locating ${ana_lib}")
	find_library(ANA_${ana_lib}_LIBRARY ${ana_lib} ${ANA_CURRENT_ROOT}/arch/${ANA_ARCH}/lib/)
	mark_as_advanced(ANA_${ana_lib}_LIBRARY)
	LIST(APPEND PSANA_LIBRARIES ${ANA_${ana_lib}_LIBRARY})
endforeach(ana_lib)

LIST(APPEND PSANA_INCLUDES ${ANA_CURRENT_ROOT}/include ${ANA_CURRENT_ROOT}/arch/${ANA_ARCH}/geninc)

