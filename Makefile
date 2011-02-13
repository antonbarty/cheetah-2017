###############################################
# 	Makefile for LCLS csPad detector version of myana
#	Anton Barty, December 2010
#	Note: need to have 
###############################################
TARGET 			= cspad_cryst
ARCH 			= x86_64-linux

MYANADIR		= myana
CSPADDIR		= cspad-gjw
HDF5DIR 		= /reg/neh/home/barty/hdf5
ROOTSYS			= /reg/g/pcds/package/root
OBJFILES		= main.o XtcRun.o

CPP				= g++ -c -g
LD 				= g++
CPP_LD_FLAGS	= -O4 -Wall
CFLAGS			= -Irelease -I$(HDF5DIR)/include
PDSLIBS			= -l acqdata -l bld -l xtcdata -l opal1kdata -l camdata -l pnccddata -l controldata -lipimbdata -lprincetondata -levrdata -lencoderdata -llusidata -lcspaddata
LD_FLAGS		= -Lrelease/build/pdsdata/lib/$(ARCH)/ -L$(HDF5DIR)/lib  $(PDSLIBS) -lhdf5 -lpthread -Wl,-rpath=./release/build/pdsdata/lib/$(ARCH)/
CFLAGS_ROOT		= $(shell $(ROOTSYS)/bin/root-config --cflags)
LDFLAGS_ROOT	= $(shell $(ROOTSYS)/bin/root-config --libs) -Wl,-rpath=$(ROOTSYS)/lib:release/build/pdsdata/lib/$(ARCH)/

all: $(TARGET)

clean:
	rm -f *.o *.gch $(TARGET)

remake: clean all

.PHONY: all clean remake

# Standard myana libraries
$(MYANADIR)/main.o: $(MYANADIR)/main.cc $(MYANADIR)/myana.hh $(MYANADIR)/main.hh
	$(CPP) $(CFLAGS) -o $(MYANADIR)/main.o $<

$(MYANADIR)/XtcRun.o: $(MYANADIR)/XtcRun.cc $(MYANADIR)/XtcRun.hh $(MYANADIR)/main.hh
	$(CPP) $(CFLAGS) -o $(MYANADIR)/XtcRun.o $<


# csPAD libraries
$(CSPADDIR)/myana_cspad-gjw.o: $(CSPADDIR)/myana_cspad-gjw.cc 
	$(CPP) $(CFLAGS) -o $(CSPADDIR)/myana_cspad-gjw.o $<

$(CSPADDIR)/CspadTemp.o: $(CSPADDIR)/CspadTemp.cc $(CSPADDIR)/CspadTemp.hh
	$(CPP) $(CFLAGS) -o $(CSPADDIR)/CspadTemp.o $<
	
$(CSPADDIR)/CspadGeometry.o: $(CSPADDIR)/CspadGeometry.cc $(CSPADDIR)/CspadGeometry.hh
	$(CPP) $(CFLAGS) -o $(CSPADDIR)/CspadGeometry.o $<
	
$(CSPADDIR)/CspadCorrector.o: $(CSPADDIR)/CspadCorrector.cc $(CSPADDIR)/CspadCorrector.hh
	$(CPP) $(CFLAGS) -o $(CSPADDIR)/CspadCorrector.o $<

$(CSPADDIR)/myana_cspad-gjw: $(MYANADIR)/main.o $(MYANADIR)/XtcRun.o $(CSPADDIR)/myana_cspad-gjw.o $(CSPADDIR)/CspadCorrector.o $(CSPADDIR)/CspadGeometry.o $(CSPADDIR)/CspadTemp.o $(CSPADDIR)/myana_cspad-gjw.o
	$(LD) $(CPP_LD_FLAGS) $(LD_FLAGS) -o $@ $^



# csPAD cleaner
cspad_cryst.o: cspad_cryst.cpp 
	$(CPP) $(CFLAGS) $<

worker.o: worker.cpp worker.h 
	$(CPP) $(CFLAGS) $<

setup.o: setup.cpp setup.h 
	$(CPP) $(CFLAGS) $<

data2d.o: data2d.cpp data2d.h 
	$(CPP) $(CFLAGS) $<

cspad_cryst: cspad_cryst.o setup.o worker.o data2d.o $(MYANADIR)/XtcRun.o $(MYANADIR)/main.o $(CSPADDIR)/CspadCorrector.o $(CSPADDIR)/CspadGeometry.o $(CSPADDIR)/CspadTemp.o
	$(LD) $(CPP_LD_FLAGS) $(LD_FLAGS) -o $@ $^



# test data
test: cspad_cryst
	#./cspad_cryst -f ~gjwillms/cfel-cspad/e40-r0549-s00-c00.xtc -n 2
	#./cspad_cryst -f ~gjwillms/cfel-cspad/e55-r0435-s00-c00.xtc -n 2
	./cspad_cryst -f ~gjwillms/cfel-cspad/e55-r0461-s00-c00.xtc -n 2

gdb: cspad_cryst
	#gdb ./cspad_cryst -f ~gjwillms/cfel-cspad/e40-r0549-s00-c00.xtc -n 2
	#gdb ./cspad_cryst -f ~gjwillms/cfel-cspad/e55-r0435-s00-c00.xtc -n 2
	gdb ./cspad_cryst -f ~gjwillms/cfel-cspad/e55-r0461-s00-c00.xtc -n 2

valgrind: cspad_cryst
	#valgrind ./cspad_cryst -f ~gjwillms/cfel-cspad/e40-r0549-s00-c00.xtc -n 2
	#valgrind ./cspad_cryst -f ~gjwillms/cfel-cspad/e55-r0435-s00-c00.xtc -n 2
	valgrind ./cspad_cryst -f ~gjwillms/cfel-cspad/e55-r0461-s00-c00.xtc -n 2

	
