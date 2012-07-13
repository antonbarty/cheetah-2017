
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/XtcFileIterator.hh"
#include "pdsdata/xtc/BldInfo.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/ProcInfo.hh"

class myLevelIter : public XtcIterator {
public:
  enum {Stop, Continue};
  myLevelIter(Xtc* xtc, unsigned depth) : XtcIterator(xtc), _depth(depth) {}

  int process(Xtc* xtc) {
    unsigned i=_depth; while (i--) printf("  ");
    Level::Type level = xtc->src.level();
    if (level == Level::Source) {
      const DetInfo& info = static_cast<const DetInfo&>(xtc->src);
      printf("%s contains %s  ext %x  dmg %x\n",
	     DetInfo::name(info),
	     TypeId::name(xtc->contains.id()),
	     xtc->extent,
	     xtc->damage.value());
    }
    else if (level == Level::Reporter) {
      const BldInfo& info = static_cast<const BldInfo&>(xtc->src);
      printf("%s contains %s  ext %x  dmg %x\n",
	     BldInfo::name(info),
	     TypeId::name(xtc->contains.id()),
	     xtc->extent,
	     xtc->damage.value());
    }
    else {
      const ProcInfo&info = static_cast<const ProcInfo&>(xtc->src);
      printf("%x/%d contains %s  ext %x  dmg %x\n",
	     info.ipAddr(),info.processId(),
	     TypeId::name(xtc->contains.id()),
	     xtc->extent,
	     xtc->damage.value());
    }
    
    if (xtc->contains.id() == TypeId::Id_Xtc) {
      myLevelIter iter(xtc,_depth+1);
      iter.iterate();
    }
    return Continue;
  }
private:
  unsigned _depth;
};

void usage(char* progname) {
  fprintf(stderr,"Usage: %s -f <filename> [-h]\n", progname);
}

int main(int argc, char* argv[]) {
  int c;
  char* xtcname=0;
  int parseErr = 0;
  unsigned damage = -1;

  while ((c = getopt(argc, argv, "hf:d:")) != -1) {
    switch (c) {
    case 'h':
      usage(argv[0]);
      exit(0);
    case 'f':
      xtcname = optarg;
      break;
    case 'd':
      damage = strtoul(optarg,NULL,0);
      break;
    default:
      parseErr++;
    }
  }
  
  if (!xtcname) {
    usage(argv[0]);
    exit(2);
  }

  int fd = open(xtcname,O_RDONLY | O_LARGEFILE);
  if (fd < 0) {
    printf("Unable to open file %s\n",xtcname);
    exit(2);
  }

  XtcFileIterator iter(fd,0x900000);
  Dgram* dg;
  unsigned long long bytes=0;
  unsigned events=0;
  unsigned damaged=0;
  while ((dg = iter.next())) {
    events++;
    if (dg->xtc.damage.value()&damage) {
      damaged++;
      printf("%s transition: time %08x/%08x  stamp %08x/%08x, dmg %08x, payloadSize 0x%x  evt %d  pos 0x%llx\n",
	     TransitionId::name(dg->seq.service()),
	     dg->seq.clock().seconds(),dg->seq.clock().nanoseconds(),
	     dg->seq.stamp().fiducials(),dg->seq.stamp().ticks(),
	     dg->xtc.damage.value(),
	     dg->xtc.sizeofPayload(),
	     events,
	     bytes);
      myLevelIter iter(&(dg->xtc),0);
      iter.iterate();
    }
    bytes += sizeof(*dg) + dg->xtc.sizeofPayload();
  }

  printf("Found %d/%d events damaged in %lld bytes\n", damaged, events, bytes);

  close(fd);
  return 0;
}
