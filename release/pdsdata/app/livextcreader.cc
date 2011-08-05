
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include "pdsdata/xtc/Dgram.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/ProcInfo.hh"
#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescV1.hh"
#include "pdsdata/ipimb/ConfigV1.hh"
#include "pdsdata/ipimb/ConfigV2.hh"
#include "pdsdata/ipimb/DataV1.hh"
#include "pdsdata/ipimb/DataV2.hh"
#include "pdsdata/encoder/ConfigV1.hh"
#include "pdsdata/encoder/DataV1.hh"
#include "pdsdata/camera/FrameV1.hh"
#include "pdsdata/camera/FrameFexConfigV1.hh"
#include "pdsdata/camera/TwoDGaussianV1.hh"
#include "pdsdata/opal1k/ConfigV1.hh"
#include "pdsdata/pnCCD/ConfigV1.hh"
#include "pdsdata/pnCCD/FrameV1.hh"
#include "pdsdata/evr/ConfigV1.hh"
#include "pdsdata/evr/ConfigV2.hh"
#include "pdsdata/evr/ConfigV3.hh"
#include "pdsdata/evr/DataV3.hh"
#include "pdsdata/control/ConfigV1.hh"
#include "pdsdata/control/PVControl.hh"
#include "pdsdata/control/PVMonitor.hh"
#include "pdsdata/epics/EpicsPvData.hh"
#include "pdsdata/epics/EpicsXtcSettings.hh"
#include "pdsdata/bld/bldData.hh"
#include "pdsdata/princeton/ConfigV1.hh"
#include "pdsdata/princeton/FrameV1.hh"

using namespace Pds;

namespace Pds {
  class LiveXtcFileIterator {
  public:
    LiveXtcFileIterator(const char* xtcname, size_t maxDgramSize) :
      _maxDgramSize(maxDgramSize), 
      _buf(new char[maxDgramSize])  
    {
      _fd = ::open(xtcname,O_RDONLY | O_LARGEFILE);
      if (_fd < 0) {
	perror("Unable to open file\n");
	exit(2);
      }
      _pfd.fd     = _fd;
      _pfd.events = POLLIN;
    }
    ~LiveXtcFileIterator() 
    {
      delete[] _buf; 
      ::close(_fd);
    }
    Dgram* next() 
    {
      Dgram& dg = *(Dgram*)_buf;
      _read(&dg, sizeof(dg));
      size_t payloadSize = dg.xtc.sizeofPayload();
      if ((payloadSize+sizeof(dg))>_maxDgramSize) {
	printf("Datagram size %zu larger than maximum: %zu\n", payloadSize+sizeof(dg), _maxDgramSize);
	return 0;
      }
      _read(dg.xtc.payload(), payloadSize);
      return &dg;
    }
  private:
    int _read(void* buf, ssize_t sz) 
    {
      char* p = (char*)buf;
      //nfds_t nfd = 1;

      ssize_t rsz = ::read(_fd, p, sz);
      p  += rsz;
      sz -= rsz;

      while(sz) {
	//	printf("Waiting for %d bytes\n",sz);
	timespec tp;
	tp.tv_sec  = 0;
	tp.tv_nsec = (long int) 1.0e8;
	nanosleep(&tp,0);

	if ((rsz = ::read(_fd, p, sz))==-1) {
	  perror("Error reading from file");
	  exit(1);
	}
	p  += rsz;
	sz -= rsz;
      }
      return p - (char*)buf;
    }
  private:
    int _fd;
    size_t   _maxDgramSize;
    char*    _buf;
    pollfd   _pfd;
  };
};

static unsigned nevents [Pds::TypeId::NumberOf+1];

class myLevelIter : public XtcIterator {
public:
  enum {Stop, Continue};
  myLevelIter(Xtc* xtc, unsigned depth) : XtcIterator(xtc), _depth(depth) {}

  int process(Xtc* xtc) {

    if (xtc->contains.id() < Pds::TypeId::NumberOf)
      ++nevents[xtc->contains.id()];
    else {
      ++nevents[Pds::TypeId::NumberOf];
      printf("Unexpected type id 0x%x\n",xtc->contains.value());
    }

    if (xtc->contains.id() == TypeId::Id_Xtc) {
      myLevelIter iter(xtc,_depth+1);
      iter.iterate();
    }
    return Continue;
  }
private:
  unsigned _depth;

  /* static private data */
  static PNCCD::ConfigV1 _pnCcdCfgList[2];  
};

PNCCD::ConfigV1 myLevelIter::_pnCcdCfgList[2] = { PNCCD::ConfigV1(), PNCCD::ConfigV1() };

void usage(char* progname) {
  fprintf(stderr,"Usage: %s -f <filename> [-h]\n", progname);
}

int main(int argc, char* argv[]) {
  int c;
  char* xtcname=0;
  int parseErr = 0;
  bool verbose = false;

  while ((c = getopt(argc, argv, "hvf:")) != -1) {
    switch (c) {
    case 'h':
      usage(argv[0]);
      exit(0);
    case 'v':
      verbose = true;
      break;
    case 'f':
      xtcname = optarg;
      break;
    default:
      parseErr++;
    }
  }
  
  if (!xtcname) {
    usage(argv[0]);
    exit(2);
  }

  memset(nevents,0,sizeof(nevents));
  unsigned ndamaged = 0;

  LiveXtcFileIterator iter(xtcname,0x900000);
  Dgram* dg;
  do {
    dg = iter.next();
    if (dg->xtc.damage.value()) ++ndamaged;
    if (dg->seq.service()!=TransitionId::L1Accept || verbose)
      printf("%s transition: time 0x%x/0x%x, payloadSize 0x%x\n",TransitionId::name(dg->seq.service()),
	     dg->seq.stamp().fiducials(),dg->seq.stamp().ticks(),dg->xtc.sizeofPayload());
    myLevelIter xiter(&(dg->xtc),0);
    xiter.iterate();
  } while (dg->seq.service()!=TransitionId::EndRun);

  for(unsigned i=0; i<Pds::TypeId::NumberOf; i++)
    printf("number of %30.30s containers : %u\n",Pds::TypeId::name(Pds::TypeId::Type(i)),nevents[i]);
  printf("number of %30.30s containers : %u\n","unknown",nevents[Pds::TypeId::NumberOf]);
  printf("number of damaged events : %u\n",ndamaged);
  return 0;
}
