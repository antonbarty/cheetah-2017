###############################################
# 	Makefile for LCLS csPad detector version of myana
#	Anton Barty, December 2010
#
#	Note: before this makefile will work it is necessary to compile 
#	the LCLS libraries.  This is done as follows:
#	> cd release/pdsdata
#	> make x86_64-linux
#	(this creates the LCLS libraries necessary for compile to complete.
#	if not done, you'll get an error such as 'can not find libacqdata.so'.
#	Change the target CPU/OS combination if working on a different system, eg: OS-X)
#
###############################################
TARGET 			= cheetah
ARCH 			= x86_64-linux



HDF5DIR 		?= /cfel/common
CHEETAHDIR      ?= /cfel/user/barty/c/cheetah
LIBCHEETAHDIR   ?= $(CHEETAHDIR)/cheetah.ib

LCLSDIR 		?= $(CHEETAHDIR)/lcls/release
MYANADIR		?= lcls/myana
CSPADDIR		?= lcls/cspad

#LCLSDIR 		= /reg/neh/home/barty/c/cspad-cryst/release
#HDF5DIR 		= /reg/neh/home/barty/hdf5

OBJFILES		= main.o XtcRun.o

CPP				= g++ -c -g -O3
#CFLAGS			= -I$(CHEETAHDIR) -I$(LIBCHEETAHDIR) -I$(CHEETAHDIR)/lcls/release -I$(HDF5DIR)/include
CFLAGS			= -Icheetah.lib/ -Ilcls/release -I$(HDF5DIR)/include

LD 				= g++
CPP_LD_FLAGS	= -O4 -Wall
PDSLIBS			= -l acqdata -l bld -l xtcdata -l opal1kdata -l camdata -l pnccddata -l controldata -lipimbdata -lprincetondata -levrdata -lencoderdata -llusidata -lcspaddata
LD_FLAGS		= -Llcls/release/build/pdsdata/lib/$(ARCH)/ -L$(HDF5DIR)/lib  $(PDSLIBS) -lhdf5 -lpthread -Wl,-rpath=$(CHEETAHDIR):$(LCLSDIR)/build/pdsdata/lib/$(ARCH)/:$(HDF5DIR)/lib

VPATH           = cheetah.lib/



all: lcls  $(TARGET)

clean:
	rm -f *.o *.gch $(TARGET)

remake: clean all
.PHONY: all clean remake

lcls:
# Standard myana 
$(MYANADIR)/main.o: $(MYANADIR)/main.cc $(MYANADIR)/myana.hh $(MYANADIR)/main.hh
	$(CPP) $(CFLAGS) -o $(MYANADIR)/main.o $<
$(MYANADIR)/XtcRun.o: $(MYANADIR)/XtcRun.cc $(MYANADIR)/XtcRun.hh $(MYANADIR)/main.hh
	$(CPP) $(CFLAGS) -o $(MYANADIR)/XtcRun.o $<
# CSPAD specific
$(CSPADDIR)/myana_cspad.o: $(CSPADDIR)/myana_cspad.cc 
	$(CPP) $(CFLAGS) -o $(CSPADDIR)/myana_cspad.o $<
$(CSPADDIR)/CspadTemp.o: $(CSPADDIR)/CspadTemp.cc $(CSPADDIR)/CspadTemp.hh
	$(CPP) $(CFLAGS) -o $(CSPADDIR)/CspadTemp.o $<
$(CSPADDIR)/CspadGeometry.o: $(CSPADDIR)/CspadGeometry.cc $(CSPADDIR)/CspadGeometry.hh
	$(CPP) $(CFLAGS) -o $(CSPADDIR)/CspadGeometry.o $<
$(CSPADDIR)/CspadCorrector.o: $(CSPADDIR)/CspadCorrector.cc $(CSPADDIR)/CspadCorrector.hh
	$(CPP) $(CFLAGS) -o $(CSPADDIR)/CspadCorrector.o $<
$(CSPADDIR)/myana_cspad: $(MYANADIR)/main.o $(MYANADIR)/XtcRun.o $(CSPADDIR)/myana_cspad.o $(CSPADDIR)/CspadCorrector.o $(CSPADDIR)/CspadGeometry.o $(CSPADDIR)/CspadTemp.o $(CSPADDIR)/myana_cspad.o
	$(LD) $(CPP_LD_FLAGS) $(LD_FLAGS) -o $@ $^



# Cheetah library
cheetah.lib: cheetah.o libcheetah.o worker.o setup.o data2d.o median.o hitfinders.o detectorObject.o detectorCorrection.o backgroundCorrection.o powder.o radialAverage.o saveFrame.o

libcheetah.o: libcheetah.cpp cheetah.h cheetahEvent.h cheetahGlobal.h detectorObject.h
	$(CPP) $(CFLAGS) $< -o $@

worker.o: worker.cpp cheetah.h cheetahEvent.h cheetahGlobal.h detectorObject.h
	$(CPP) $(CFLAGS) $<

setup.o: setup.cpp cheetah.h cheetahGlobal.h cheetahEvent.h detectorObject.h
	$(CPP) $(CFLAGS) $<

data2d.o: data2d.cpp cheetah.h data2d.h cheetahEvent.h cheetahGlobal.h
	$(CPP) $(CFLAGS) $<

median.o: median.cpp cheetah.h median.h cheetahEvent.h cheetahGlobal.h 
	$(CPP) $(CFLAGS) $<

hitfinders.o: hitfinders.cpp cheetah.h hitfinders.h cheetahEvent.h cheetahGlobal.h detectorObject.h
	$(CPP) $(CFLAGS) $<

detectorObject.o: detectorObject.cpp cheetah.h cheetahEvent.h cheetahGlobal.h detectorObject.h
	$(CPP) $(CFLAGS) $<

detectorCorrection.o: detectorCorrection.cpp cheetah.h cheetahEvent.h cheetahGlobal.h detectorObject.h
	$(CPP) $(CFLAGS) $<

backgroundCorrection.o: backgroundCorrection.cpp cheetah.h cheetahEvent.h cheetahGlobal.h detectorObject.h
	$(CPP) $(CFLAGS) $<

powder.o: powder.cpp cheetah.h cheetahEvent.h cheetahGlobal.h detectorObject.h
	$(CPP) $(CFLAGS) $<
    
radialAverage.o: radialAverage.cpp cheetah.h cheetahEvent.h cheetahGlobal.h detectorObject.h
	$(CPP) $(CFLAGS) $<

saveFrame.o: saveFrame.cpp cheetah.h cheetahEvent.h cheetahGlobal.h detectorObject.h
	$(CPP) $(CFLAGS) $<


cheetah.o: cheetah.cpp cheetah.h cheetahEvent.h cheetahGlobal.h detectorObject.h
	$(CPP) $(CFLAGS) $<

cheetah: cheetah.o libcheetah.o setup.o median.o hitfinders.o detectorObject.o powder.o radialAverage.o saveFrame.o detectorCorrection.o backgroundCorrection.o worker.o data2d.o $(MYANADIR)/XtcRun.o $(MYANADIR)/main.o $(CSPADDIR)/CspadCorrector.o $(CSPADDIR)/CspadGeometry.o $(CSPADDIR)/CspadTemp.o
	$(LD) $(CPP_LD_FLAGS) $(LD_FLAGS) -o $@ $^

	
