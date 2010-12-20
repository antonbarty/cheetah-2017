TARGET                  = xtcdump
ARCH                    = i386-linux-opt
ROOTSYS                 = /reg/g/pcds/package/root
PCDS_DIR            	= /reg/neh/home/taw/myana/release
#PCDS_DIR            	= /reg/neh/home/barty/schotte/xtcdump/release
PCDS_INCLUDE            = $(PCDS_DIR)
PCDS_PACKAGE            = $(PCDS_DIR)/build/pdsdata/lib/$(ARCH)/
OBJFILES                = main.o XtcRun.o
CPP                     = g++ -c
LD                      = g++
CPP_LD_FLAGS            = -m32 -O4 -Wall
CFLAGS					= -I$(PCDS_INCLUDE)
PDSLIBS                 = -lacqdata -lbld -lxtcdata -lopal1kdata -lcamdata -lpnccddata -lcontroldata -lipimbdata -lprincetondata -levrdata -lencoderdata -llusidata -lcspaddata
LD_FLAGS                = $(PDSLIBS) -L$(PCDS_PACKAGE)
CFLAGS_ROOT             = $(shell $(ROOTSYS)/bin/root-config --cflags)
LDFLAGS_ROOT            = $(shell $(ROOTSYS)/bin/root-config --libs) -Wl,-rpath=$(ROOTSYS)/lib:$(PCDS_PACKAGE)

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

	
