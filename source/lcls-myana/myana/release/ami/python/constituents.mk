libnames := pyami

libsrcs_pyami := pyami.cc
libsrcs_pyami += Discovery.cc
libsrcs_pyami += Client.cc
libincs_pyami := python/include/python2.5
liblibs_pyami := pdsdata/xtcdata pdsdata/acqdata
liblibs_pyami += pdsdata/camdata pdsdata/opal1kdata
liblibs_pyami += pdsdata/pulnixdata pdsdata/princetondata
liblibs_pyami += pdsdata/pnccddata pdsdata/ipimbdata
liblibs_pyami += pdsdata/evrdata pdsdata/encoderdata
liblibs_pyami += pdsdata/gsc16aidata
liblibs_pyami += pdsdata/controldata pdsdata/epics 
liblibs_pyami += pdsdata/cspaddata pdsdata/lusidata
liblibs_pyami += ami/service ami/data ami/server ami/client
liblibs_pyami += qt/QtCore
libslib_pyami := $(USRLIBDIR)/rt gomp
