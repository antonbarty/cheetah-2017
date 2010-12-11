
TARGET 			= xtcdump
ARCH 			= i386-linux-opt
ROOTSYS			= /reg/g/pcds/package/root
OBJFILES		= main.o XtcRun.o
CPP				= g++ -c
LD 				= g++
CPP_LD_FLAGS	= -m32 -O4 -Wall
CFLAGS			= -Irelease
PDSLIBS			= -l acqdata -l bld -l xtcdata -l opal1kdata -l camdata -l pnccddata -l controldata -lipimbdata -lprincetondata -levrdata -lencoderdata -llusidata -lcspaddata
LD_FLAGS		= $(PDSLIBS) -Lrelease/build/pdsdata/lib/$(ARCH)/ 
CFLAGS_ROOT		= $(shell $(ROOTSYS)/bin/root-config --cflags)
LDFLAGS_ROOT	= $(shell $(ROOTSYS)/bin/root-config --libs) -Wl,-rpath=$(ROOTSYS)/lib:release/build/pdsdata/lib/$(ARCH)/

all: $(TARGET)

clean:
	rm -f *.o *.gch $(TARGET)

remake: clean all

.PHONY: all clean remake

main.o: main.cc myana.hh main.hh
	$(CPP) $(CPP_LD_FLAGS) $(CFLAGS) $(CFLAGS_ROOT) $<

XtcRun.o: XtcRun.cc XtcRun.hh main.hh
	$(CPP) $(CPP_LD_FLAGS) $(CFLAGS) $(CFLAGS_ROOT) $<

xtcdump.o: xtcdump.cc myana.hh main.hh
	$(CPP) $(CPP_LD_FLAGS) $(CFLAGS) $(CFLAGS_ROOT) $<

xtcdump: main.o xtcdump.o XtcRun.o
	$(LD) $(CPP_LD_FLAGS) $(LD_FLAGS) $(LDFLAGS_ROOT) -o $@ $^

test: xtcdump
	./xtcdump -f ~schotte/xpp23410/sample_files/XTC/e66-r0039-s00-c00.xtc -n 21

	
