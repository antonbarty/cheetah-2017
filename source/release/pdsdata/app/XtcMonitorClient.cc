#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#ifdef _POSIX_MESSAGE_PASSING
#include <mqueue.h>
#endif

#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/ProcInfo.hh"
#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/Dgram.hh"
#include "XtcMonitorClient.hh"
#include "XtcMonitorMsg.hh"

#include <poll.h>

static const unsigned MaxClients=10;

enum {PERMS_IN  = S_IRUSR|S_IRGRP|S_IROTH};
enum {PERMS_OUT  = S_IWUSR|S_IWGRP|S_IWOTH};
enum {OFLAGS = O_RDONLY};

static mqd_t _openQueue(const char* name, unsigned flags, unsigned perms,
                        bool lwait=true)
{
  struct mq_attr mymq_attr;
  mqd_t queue;
  while(1) {
    queue = mq_open(name, flags, perms, &mymq_attr);
    if (queue == (mqd_t)-1) {
      char b[128];
      sprintf(b,"mq_open %s",name);
      perror(b);
      sleep(1);
      if (!lwait) break;
    }
    else {
      printf("Opened queue %s (%d)\n",name,queue);
      break;
    }
  }
  return queue;
}

static void _flushQueue(mqd_t q)
{
  // flush the queues just to be sure they are empty.
  XtcMonitorMsg m;
  unsigned priority;
  struct mq_attr mymq_attr;
  mymq_attr.mq_curmsgs=0;
  do {
    mq_getattr(q, &mymq_attr);
    if (mymq_attr.mq_curmsgs)
      mq_receive(q, (char*)&m, sizeof(m), &priority);
  } while (mymq_attr.mq_curmsgs);
}

namespace Pds {
  class DgramHandler {
  public:
    DgramHandler(XtcMonitorClient& client, 
		 mqd_t trq, mqd_t evqin, mqd_t* evqout, unsigned ev_index,
	      const char* tag, char* myShm) :
      _client(client), 
      _trq(trq), _evqin(evqin), _evqout(evqout), _ev_index(ev_index),
      _tag(tag), _shm(myShm) 
    {
      _tmo.tv_sec = _tmo.tv_nsec = 0;
    }
    ~DgramHandler() {}
  public:
    bool event     () { return _handle(_evqin,_evqout,_ev_index); }
    bool transition() { return _handle(_trq  ,NULL   ,0); }
  private:
    bool _handle(mqd_t iq, mqd_t* oq, unsigned ioq)
    {
      XtcMonitorMsg myMsg;
      unsigned priority;
      if (mq_receive(iq, (char*)&myMsg, sizeof(myMsg), &priority) < 0) {
	perror("mq_receive buffer");
	return false;
      } 
      else {
	int i = myMsg.bufferIndex();
	if ( (i>=0) && (i<myMsg.numberOfBuffers())) {
	  Dgram* dg = (Dgram*) (_shm + (myMsg.sizeOfBuffers() * i));
	  if (_client.processDgram(dg))
	    return false;
	  if (oq!=NULL)
	    while (mq_timedsend(oq[ioq], (const char *)&myMsg, sizeof(myMsg), priority, &_tmo)) {
	      if (oq[++ioq]==-1) {
		char qname[128];
		XtcMonitorMsg::eventOutputQueue(_tag, ioq, qname);
		oq[ioq] = _openQueue(qname, O_WRONLY, PERMS_OUT, false);
	      }
	    }
	}
	else {
	  fprintf(stderr, "ILLEGAL BUFFER INDEX %d\n", i);
	  return false;
	}
      }
      return true;
    }
  private:
    XtcMonitorClient& _client;
    mqd_t             _trq;
    mqd_t             _evqin;
    mqd_t*            _evqout;
    unsigned          _ev_index;
    const char*       _tag;
    char*             _shm;
    timespec          _tmo;
  };
};

using namespace Pds;

int XtcMonitorClient::processDgram(Dgram* dg) {
  printf("%s transition: time 0x%x/0x%x, payloadSize 0x%x\n",TransitionId::name(dg->seq.service()),
      dg->seq.stamp().fiducials(),dg->seq.stamp().ticks(),dg->xtc.sizeofPayload());
  return 0;
}

int XtcMonitorClient::run(const char * tag, int tr_index) 
{ return run(tag, tr_index, tr_index); }

int XtcMonitorClient::run(const char * tag, int tr_index, int ev_index) {
  int error = 0;
  char* qname             = new char[128];

  unsigned priority = 0;
  XtcMonitorMsg myMsg;

  mqd_t myOutputEvQueues[MaxClients];
  memset(myOutputEvQueues, -1, sizeof(myOutputEvQueues));

  XtcMonitorMsg::eventOutputQueue(tag,ev_index,qname);
  myOutputEvQueues[ev_index] = _openQueue(qname, O_WRONLY, PERMS_OUT);
  if (myOutputEvQueues[ev_index] == (mqd_t)-1)
    error++;

  XtcMonitorMsg::eventInputQueue(tag,ev_index,qname);
  mqd_t myInputEvQueue = _openQueue(qname, O_RDONLY, PERMS_IN);
  if (myInputEvQueue == (mqd_t)-1)
    error++;

  XtcMonitorMsg::discoveryQueue(tag,qname);
  mqd_t discoveryQueue = _openQueue(qname, O_WRONLY, PERMS_OUT);
  if (discoveryQueue == (mqd_t)-1)
    error++;

  XtcMonitorMsg::transitionInputQueue(tag,tr_index,qname);
  mqd_t myInputTrQueue = _openQueue(qname, O_RDONLY, PERMS_IN);
  if (myInputTrQueue == (mqd_t)-1)
    error++;
  _flushQueue(myInputTrQueue);

  if (error) {
    fprintf(stderr, "Could not open at least one message queue!\n");
    fprintf(stderr, "tag %s, tr_index %d, ev_index %d\n",tag,tr_index,ev_index);
    return error;
  }

  //
  //  Request initialization
  //
  myMsg.bufferIndex(tr_index);
  if (mq_send(discoveryQueue, (const char *)&myMsg, sizeof(myMsg), priority)) {
    perror("mq_send tr queue");
    error++;
  }

  //
  //  Initialize shared memory from first message
  //  Seek the Map transition
  //
  char* myShm = 0;
  bool init=false;
  do {
    XtcMonitorMsg myMsg;
    unsigned priority;
    if (mq_receive(myInputTrQueue, (char*)&myMsg, sizeof(myMsg), &priority) < 0) {
      perror("mq_receive buffer");
      return ++error;
    } 
    else {
      if (!init) {
        init = true;
        unsigned sizeOfShm = myMsg.numberOfBuffers() * myMsg.sizeOfBuffers();
        unsigned pageSize  = (unsigned)sysconf(_SC_PAGESIZE);
        unsigned remainder = sizeOfShm % pageSize;
        if (remainder)
          sizeOfShm += pageSize - remainder;

	XtcMonitorMsg::sharedMemoryName(tag, qname);
        printf("Opening shared memory %s of size 0x%x (0x%x * 0x%x)\n",
            qname,sizeOfShm,myMsg.numberOfBuffers(),myMsg.sizeOfBuffers());

        int shm = shm_open(qname, OFLAGS, PERMS_IN);
        if (shm < 0) perror("shm_open");
        myShm = (char*)mmap(NULL, sizeOfShm, PROT_READ, MAP_SHARED, shm, 0);
        if (myShm == MAP_FAILED) perror("mmap");
        else printf("Shared memory at %p\n", (void*)myShm);
      }

      int i = myMsg.bufferIndex();
      if ( (i>=0) && (i<myMsg.numberOfBuffers())) {
        Dgram* dg = (Dgram*) (myShm + (myMsg.sizeOfBuffers() * i));
        if (dg->seq.service()==TransitionId::Map)
          if (!processDgram(dg))
            break;
      }
    }
  } while(1);

  //
  //  Handle all transitions first, then events
  //
  pollfd pfd[2];
  pfd[0].fd      = myInputTrQueue;
  pfd[0].events  = POLLIN | POLLERR;
  pfd[0].revents = 0;
  pfd[1].fd      = myInputEvQueue;
  pfd[1].events  = POLLIN | POLLERR;
  pfd[1].revents = 0;
  int nfd = 2;

  DgramHandler handler(*this,
		       myInputTrQueue,
		       myInputEvQueue,myOutputEvQueues,ev_index,
		       tag,myShm);

  while (!error) {
    if (::poll(pfd, nfd, -1) > 0) {
      if (pfd[0].revents & POLLIN) { // Transition
	if (!handler.transition()) error++;
      }
      else if (pfd[1].revents & POLLIN) { // Event
	if (!handler.event     ()) error++;
      }
    }
  }
  return error;
}

