###############################################
# 	Makefile for LCLS csPad detector version of myana
#	Anton Barty, December 2010
#
#	Note: before this makefile will work it is necessary to compile 
#	the LCLS libraries.  This is done as follows:
#	> cd lcls/myana
#	> cd release/pdsdata
#	> make x86_64-linux
#	
#	This creates the LCLS libraries necessary for compile to complete.
#	If there are complation errors, fix them before proceeding further.
#	If not done, you'll get an error such as 'can not find libacqdata.so'.
#	
#	It may also be necessary to change the paths to HDF5 and LCLSDIR depending on 
#	where things are on the target system. 
#
###############################################
TARGET 			= cheetah
ARCH 			= x86_64-linux
#ARCH 			= i386-linux


# LCLS
HDF5DIR			?= /specify/this/elsewhere
CHEETAHDIR      ?= /specify/this/elsewhere
CRYSTFELDIR		?= /specify/this/elsewhere


# cfelsgi
#HDF5DIR 		?= /cfel/common
#CHEETAHDIR      ?= /cfel/user/barty/c/cheetah-test


LIBCHEETAHDIR   ?= $(CHEETAHDIR)/cheetah.lib
MYANADIR		?= lcls/myana
LCLSDIR 		?= $(CHEETAHDIR)/source/lcls/myana/release
#CSPADDIR		?= lcls/cspad

#OBJFILES		= main.o XtcRun.o

CPP				= g++ -c -g -O3 -fPIC
#CFLAGS			= -I$(CHEETAHDIR) -I$(LIBCHEETAHDIR) -I$(MYANADIR) -I$(LCLSDIR) -I$(HDF5DIR)/include
CFLAGS			= -Icheetah.lib/ -Ilcls/myana/release/ -I$(HDF5DIR)/include 

LD 				= g++
CPP_LD_FLAGS	= -O4 -Wall

PDSLIBS			= -l acqdata -l bld -l xtcdata -l opal1kdata -l camdata -l pnccddata -l controldata -lipimbdata -lprincetondata -levrdata -lencoderdata -lgsc16aidata -llusidata -lcspaddata -lindexdata -lanadata -lcspad2x2data -lphasicsdata -ltimepixdata -loceanopticsdata -lflidata

LD_FLAGS		= -Llcls/myana/release/build/pdsdata/lib/$(ARCH)/ -L$(HDF5DIR)/lib  -Wl,-rpath=$(CHEETAHDIR):$(LCLSDIR)/build/pdsdata/lib/$(ARCH)/:$(HDF5DIR)/lib $(PDSLIBS) -lhdf5 -lpthread

VPATH           = cheetah.lib/



all:  $(TARGET)

clean:
	rm -f *.o *.gch *.so* $(TARGET)

remake: clean all
.PHONY: all clean remake

lcls:
# Standard myana 
$(MYANADIR)/main.o: $(MYANADIR)/main.cc $(MYANADIR)/myana.hh $(MYANADIR)/main.hh
	$(CPP) $(CFLAGS) -o $(MYANADIR)/main.o $<
#$(MYANADIR)/XtcRun.o: $(MYANADIR)/XtcRun.cc $(MYANADIR)/XtcRun.hh $(MYANADIR)/main.hh
#	$(CPP) $(CFLAGS) -o $(MYANADIR)/XtcRun.o $<
$(MYANADIR)/SplitEventQ.o: $(MYANADIR)/SplitEventQ.cc $(MYANADIR)/SplitEventQ.hh $(MYANADIR)/main.hh
	$(CPP) $(CFLAGS) -o $(MYANADIR)/SplitEventQ.o $<
	
myana:
$(MYANADIR)/myana:  $(MYANADIR)/SplitEventQ.cc $(MYANADIR)/SplitEventQ.hh $(MYANADIR)/main.hh $(MYANADIR)/myana.cc
	$(CPP) $(CFLAGS) -o $< $(MYANADIR)/myana.cc
$(MYANADIR)/myana_morefeatures:  $(MYANADIR)/SplitEventQ.cc $(MYANADIR)/SplitEventQ.hh $(MYANADIR)/main.hh $(MYANADIR)/myana.cc
	$(CPP) $(CFLAGS) -o $< $(MYANADIR)/myana_morefeatures.cc

	

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

cheetah-myana.o: cheetah-myana.cpp cheetah.h cheetahEvent.h cheetahGlobal.h detectorObject.h
	$(CPP) $(CFLAGS) $<

# Cheetah
cheetah: cheetah-myana.o libcheetah.o setup.o median.o hitfinders.o detectorObject.o powder.o radialAverage.o saveFrame.o detectorCorrection.o backgroundCorrection.o worker.o data2d.o  $(MYANADIR)/main.o 
	$(LD) $(CPP_LD_FLAGS) $(LD_FLAGS) -o $@ $^

libcheetah: backgroundCorrection.o data2d.o detectorCorrection.o detectorObject.o hitfinders.o libcheetah.o median.o powder.o radialAverage.o saveFrame.o setup.o worker.o
	gcc -shared -Wl,-soname,libcheetah.so.1 -o libcheetah.so.1.0 backgroundCorrection.o data2d.o detectorCorrection.o detectorObject.o hitfinders.o libcheetah.o median.o powder.o radialAverage.o saveFrame.o setup.o worker.o 