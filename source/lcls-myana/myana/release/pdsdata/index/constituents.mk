libnames := indexdata
libsrcs_indexdata := src/IndexList.cc src/XtcIterL1Accept.cc src/IndexFileStruct.cc src/IndexFileReader.cc  src/IndexChunkReader.cc  src/IndexSliceReader.cc 

tgtnames = xtcindex xtcanalyze xtcanalyzeone

#CXXFLAGS += -pthread -m32 -I/reg/g/pcds/package/root/include

#LXFLAGS += -L/reg/g/pcds/package/root/lib -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -pthread -lm -ldl -rdynamic

tgtsrcs_xtcindex := src/xtcindex.cc 
tgtlibs_xtcindex := pdsdata/xtcdata pdsdata/acqdata pdsdata/epics pdsdata/pnccddata pdsdata/bld pdsdata/controldata pdsdata/evrdata pdsdata/ipimbdata pdsdata/indexdata
tgtslib_xtcindex := $(USRLIBDIR)/rt

tgtsrcs_xtcanalyze := src/xtcanalyze.cc 
tgtlibs_xtcanalyze := pdsdata/xtcdata pdsdata/acqdata pdsdata/epics pdsdata/pnccddata pdsdata/bld pdsdata/controldata pdsdata/evrdata pdsdata/ipimbdata pdsdata/indexdata pdsdata/anadata
tgtslib_xtcanalyze := $(USRLIBDIR)/rt

tgtsrcs_xtcanalyzeone := src/xtcanalyzeone.cc 
tgtlibs_xtcanalyzeone := pdsdata/xtcdata pdsdata/acqdata pdsdata/epics pdsdata/pnccddata pdsdata/bld pdsdata/controldata pdsdata/evrdata pdsdata/ipimbdata pdsdata/indexdata
tgtslib_xtcanalyzeone := $(USRLIBDIR)/rt
