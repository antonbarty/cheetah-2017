#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "pdsdata/xtc/Dgram.hh"
#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/app/XtcMonitorClient.hh"

#include "Event.hh"
#include "Server.hh"

#include <map>

using namespace Pds;

class MyIterator : public XtcIterator {
public:
  MyIterator(Dgram& dg) : XtcIterator(&dg.xtc), _dg(dg) { iterate(); }
  ~MyIterator() {}
public:
  int process(Xtc* xtc) {
    if (xtc->contains.value()==TypeId::Id_Xtc) {
      iterate(xtc);
      return 1;
    }

    if (xtc->src.level()==Pds::Level::Source && xtc->damage.value()==0)
      _event->add(*xtc);

    return 0;
  }
private:
  Dgram& _dg;
  Event  _event;
};

class MyXtcMonitorClient : public XtcMonitorClient {
public:
  MyXtcMonitorClient(Server& srv) : _srv(srv) {}
public:
  virtual void processDgram(Dgram* dg) {
    printf("%s transition: time 0x%x/0x%x, payloadSize 0x%x\n",TransitionId::name(dg->seq.service()),
	   dg->seq.stamp().fiducials(),dg->seq.stamp().ticks(),dg->xtc.sizeofPayload());

    MyIterator iter(*dg);
    _srv.event(*dg,iter.event());
  };
private:
  Server& _srv;
};

void usage(char* progname) {
  fprintf(stderr,"Usage: %s [-p <partitionTag>] [-h]\n", progname);
}

int main(int argc, char* argv[]) {
  int c;
  char partitionTag[128] = "";

  while ((c = getopt(argc, argv, "?hp:")) != -1) {
    switch (c) {
    case 'p':
      strcpy(partitionTag, optarg);
      break;
    case '?':
    case 'h':
    default:
      usage(argv[0]);
      exit(0);
    }
  }

  //  Setup socket/thread to listen to controller
  Server srv;
  MyXtcMonitorClient myClient(srv);
  
  //  Setup channel access thread
  int result = myClient.run(partitionTag);
  fprintf(stderr, "myClient returned: %d\n", result); 
  return result;
}
