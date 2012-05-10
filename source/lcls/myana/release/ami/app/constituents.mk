ifneq ($(findstring x86_64-linux,$(tgt_arch)),)
qtincdir  := qt/include_64
else
qtincdir  := qt/include
endif

# List targets (if any) for this package
tgtnames := ami test

# List source files for each target
tgtsrcs_ami := ami.cc AmiApp.cc AmiApp.hh
tgtsrcs_test := test.cc

# List system libraries (if any) needed by exe_a as <dir>/<lib>. 
# Note that <lib> is the name of the library, not of the file: i.e.
# <lib> for 'libc.so' is 'c'. Low level first.
tgtslib_ami := $(USRLIBDIR)/rt
tgtslib_test := $(USRLIBDIR)/rt

#
# Need all pdsdata libraries to support dynamic linking of plug-in modules
#
tgtlibs_ami := pdsdata/xtcdata pdsdata/acqdata pdsdata/timepixdata
tgtlibs_ami += pdsdata/camdata pdsdata/opal1kdata
tgtlibs_ami += pdsdata/pulnixdata pdsdata/princetondata
tgtlibs_ami += pdsdata/pnccddata pdsdata/ipimbdata
tgtlibs_ami += pdsdata/evrdata pdsdata/encoderdata
tgtlibs_ami += pdsdata/gsc16aidata
tgtlibs_ami += pdsdata/controldata pdsdata/epics 
tgtlibs_ami += pdsdata/cspaddata pdsdata/lusidata pdsdata/appdata
tgtlibs_ami += pdsdata/cspad2x2data
tgtlibs_ami += pdsdata/fexampdata
tgtlibs_ami += pdsdata/phasicsdata pdsdata/oceanopticsdata pdsdata/flidata
tgtlibs_ami += ami/service ami/data ami/server ami/event ami/client ami/app
tgtincs_ami := $(qtincdir)
ifneq ($(findstring x86_64-linux,$(tgt_arch)),)
  tgtlibs_ami += qt/QtCore
else
  tgtlibs_ami += qt/QtCore
endif

# List system include directories (if any) needed by exe_a as <incdir>.
# tgtsinc_exe_a := /usr/include

# List libraries (if any) for this package
libnames := app

# List source files for each library
#libsrcs_app := $(filter-out Agent.cc ami_agent.cc test.cc ami.cc,$(wildcard *.cc))
libsrcs_app := $(filter-out test.cc ami.cc,$(wildcard *.cc))
# libsrcs_lib_b := src_6.cc

# List special include directories (if any) needed by lib_a as
# <project>/<incdir>. Note that the top level release directory is
# already in the search path.
# libincs_lib_a := prj_x/include/Linux

# List system include directories (if any) needed by lib_a as <incdir>.
# libsinc_lib_a := /usr/include
