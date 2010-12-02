#include "pdsdata/app/XtcMonitorServer.hh"

#include "pdsdata/xtc/ProcInfo.hh"
#include "pdsdata/xtc/Dgram.hh"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <queue>

using namespace Pds;

//static char* dgramBuffer;

class MyMonitorServer : public XtcMonitorServer {
public:
  MyMonitorServer(const char* tag,
		  unsigned sizeofBuffers, 
		  unsigned numberofEvBuffers, 
		  unsigned numberofClients,
		  unsigned sequenceLength) :
    XtcMonitorServer(tag,
		     sizeofBuffers,
		     numberofEvBuffers,
		     numberofClients,
		     sequenceLength) 
  {
    //  sum of client queues (nEvBuffers) + clients + transitions + shuffleQ
    unsigned depth = 2*numberofEvBuffers+XtcMonitorServer::numberofTrBuffers+numberofClients;
    for(unsigned i=0; i<depth; i++)
      _pool.push(reinterpret_cast<Dgram*>(new char[sizeofBuffers]));
  }
  ~MyMonitorServer() 
  {
    while(!_pool.empty()) {
      delete _pool.front();
      _pool.pop();
    }
  }
public:
  XtcMonitorServer::Result events(Dgram* dg) {
    if (XtcMonitorServer::events(dg) == XtcMonitorServer::Handled)
      _deleteDatagram(dg);
    return XtcMonitorServer::Deferred;
  }
  Dgram* newDatagram() 
  { 
    Dgram* dg = _pool.front(); 
    _pool.pop(); 
    return dg; 
  }
  void   deleteDatagram(Dgram* dg) { _deleteDatagram(dg); }
private:
  void  _deleteDatagram(Dgram* dg)
  {
    _pool.push(dg); 
  }
private:
  std::queue<Dgram*> _pool;
};

static MyMonitorServer* apps;


static Dgram* next(int fd, int sizeOfBuffers) 
{
  Dgram* ndg = apps->newDatagram();
  char* b = reinterpret_cast<char*>(ndg);
  Dgram& dg = *ndg;
  unsigned header = sizeof(dg);
  int sz;
  if ((sz=::read(fd, b, header)) != int(header)) {
    if (sz == -1)
      printf("Error reading event header : %s\n",strerror(errno));
    apps->deleteDatagram(ndg);
    return 0;
  }

  unsigned payloadSize = dg.xtc.sizeofPayload();
  if ((payloadSize+header)>unsigned(sizeOfBuffers)) {
    printf("Dgram size 0x%x larger than maximum: 0x%x\n",
	   (unsigned)payloadSize+(unsigned)sizeof(dg), 
	   sizeOfBuffers); 
    apps->deleteDatagram(ndg);
    return 0;
  }
    
  if ((sz=::read(fd, b+header, payloadSize)) != int(payloadSize)) {
    printf("Read payload found %d/%d bytes\n",sz,payloadSize);
    apps->deleteDatagram(ndg);
    return 0;
  }

  return ndg;
}

static void printTransition(const Dgram* dg)
{
  printf("%18s transition: time %08x/%08x, payloadSize 0x%08x dmg 0x%x\n",
	 TransitionId::name(dg->seq.service()),
	 dg->seq.stamp().fiducials(),dg->seq.stamp().ticks(),
	 dg->xtc.sizeofPayload(),
	 dg->xtc.damage.value());
}

//
//  Insert a simulated transition
//
static Dgram* insert(TransitionId::Value tr)
{
  Dgram* dg = apps->newDatagram();
  new((void*)&dg->seq) Sequence(Sequence::Event, tr, ClockTime(0,0), TimeStamp(0,0,0,0));
  new((char*)&dg->xtc) Xtc(TypeId(TypeId::Id_Xtc,0),ProcInfo(Level::Event,0,0));
  printTransition(dg);
  return dg;
}
      
long long int timeDiff(struct timespec* end, struct timespec* start) {
  long long int diff;
  diff =  (end->tv_sec - start->tv_sec) * 1000000000;
  diff += end->tv_nsec;
  diff -= start->tv_nsec;
  return diff;
}

void usage(char* progname) {
  fprintf(stderr,"Usage: %s -f <filename> -n <numberOfBuffers> -s <sizeOfBuffers> [-r <ratePerSec>] [-p <partitionTag>] [-c <# clients>] [-l] [-h]\n", progname);
}

void sigfunc(int sig_no) {
   delete apps;
   exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
  int c;
  char* xtcname=0;
  char* partitionTag = 0;
  int rate = 1;
  unsigned nclients = 1;
  bool loop = false;
  bool verbose = false;
  bool veryverbose = false;
  int numberOfBuffers = 0;
  unsigned sizeOfBuffers = 0;
  unsigned sequenceLength = 1;
  struct timespec start, now, sleepTime;
  (void) signal(SIGINT, sigfunc);

  while ((c = getopt(argc, argv, "hf:r:n:s:p:lvVc:S:")) != -1) {
    switch (c) {
    case 'h':
      usage(argv[0]);
      exit(0);
    case 'f':
      xtcname = optarg;
      break;
    case 'r':
      sscanf(optarg, "%d", &rate);
      break;
    case 'n':
      sscanf(optarg, "%d", &numberOfBuffers);
      break;
    case 'S':
      sscanf(optarg, "%d", &sequenceLength);
      break;
    case 's':
      sizeOfBuffers = (unsigned) strtoul(optarg, NULL, 0);
      break;
    case 'p':
      partitionTag = optarg;
      break;
    case 'c':
      nclients = strtoul(optarg, NULL, 0);
      break;
    case 'l':
      loop = true;
      printf("Enabling infinite looping\n");
      break;
    case 'v':
      verbose = true;
      break;
    case 'V':
      verbose = true;
      veryverbose = true;
      break;
    default:
      fprintf(stderr, "I don't understand %c!\n", c);
      usage(argv[0]);
      exit(0);
    }
  }
  
  if (!xtcname || !sizeOfBuffers || !numberOfBuffers) {
    usage(argv[0]);
    printf("rate %d, numb %d, size %d, partition %s\n", rate, numberOfBuffers, sizeOfBuffers, partitionTag);
    exit(2);
  }

  int fd = ::open(xtcname,O_LARGEFILE,O_RDONLY);
  if (fd == -1) {
    char s[120];
    sprintf(s, "Unable to open file %s ", xtcname);
    perror(s);
    exit(2);
  }

  long long int period = 1000000000 / rate;
  sleepTime.tv_sec = 0;
  long long int busyTime = period;

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

  apps = new MyMonitorServer(partitionTag,
			     sizeOfBuffers, 
			     numberOfBuffers, 
			     nclients,
			     sequenceLength);
  
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
  printf("Opening shared memory took %lld nanonseconds.\n", timeDiff(&now, &start));

  do {

    apps->events(insert(TransitionId::Map));

    Dgram* dg;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

    while ((dg = next(fd,sizeOfBuffers))) {
      apps->events (dg);
      //      apps->routine();
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
      busyTime = timeDiff(&now, &start);
      if (dg->seq.service() != TransitionId::L1Accept)
	printTransition(dg);
      else if (verbose)
	printf("%18s transition: time %08x/%08x, payloadSize 0x%08x, raw rate %8.3f Hz%c",
	       TransitionId::name(dg->seq.service()),
	       dg->seq.stamp().fiducials(),dg->seq.stamp().ticks(),
	       dg->xtc.sizeofPayload(), 1.e9/busyTime,
	       veryverbose ? '\n' : '\r');
      if (period > busyTime) {
	sleepTime.tv_nsec = period - busyTime;
	if (nanosleep(&sleepTime, &now)<0) perror("nanosleep");
      }
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    }

    apps->events(insert(TransitionId::Unconfigure));
    apps->events(insert(TransitionId::Unmap));

    if (loop) {
      ::close(fd);
      fd = ::open(xtcname,O_LARGEFILE,O_RDONLY);
      if (fd == -1) {
	char s[120];
	sprintf(s, "Unable to open file %s ", xtcname);
	perror(s);
	exit(2);
      }
    }

  } while(loop);

  ::close(fd);
  sigfunc(0);

  return 0;
}
