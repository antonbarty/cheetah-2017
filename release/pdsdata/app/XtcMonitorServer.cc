#include "pdsdata/app/XtcMonitorServer.hh"

#include "pdsdata/xtc/Dgram.hh"

#include <unistd.h>
#ifdef _POSIX_MESSAGE_PASSING
#include <mqueue.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/prctl.h>

using std::queue;
using std::stack;

//
//  Recover any shared memory buffer older than 10 seconds
//
static const unsigned TMO_SEC = 10;

#define PERMS (S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH)
#define OFLAGS (O_CREAT|O_RDWR)

namespace Pds {
  class ShMsg {
  public:
    ShMsg() {}
    ShMsg(const XtcMonitorMsg&  m,
	  Dgram* dg) : _m(m), _dg(dg) {}
    ~ShMsg() {}
  public:
    const XtcMonitorMsg&  msg() const { return _m; }
    Dgram* dg () const { return _dg; }
  private:
    XtcMonitorMsg _m;
    Dgram*        _dg;
  };

  class EventSequence {
  public:
    EventSequence(unsigned n) :
      _dgram  (new Dgram*[n]),
      _current(0),
      _depth  (n) {}
    ~EventSequence() { delete[] _dgram; }
  public:
    bool     complete()           const { return _current==_depth; }
    Dgram*   dgram   (unsigned i) const { return _dgram[i]; }
    unsigned current ()           const { return _current; }
    unsigned depth   ()           const { return _depth; }
  public:
    void insert(Dgram* dg) { _dgram[_current++] = dg; }
    void clear () { _current = 0; }
  private:
    Dgram**  _dgram;
    unsigned _current;
    unsigned _depth;
  };
};

using namespace Pds;

XtcMonitorServer::XtcMonitorServer(const char* tag,
				   unsigned sizeofBuffers, 
				   unsigned numberofEvBuffers,
				   unsigned numberofClients,
				   unsigned sequenceLength) :
  _tag              (tag),
  _sizeOfBuffers    (sizeofBuffers),
  _numberOfEvBuffers(numberofEvBuffers),
  _numberOfClients  (numberofClients),
  _sequence(new EventSequence(sequenceLength))
{
  _myMsg.numberOfBuffers(numberofEvBuffers+numberofTrBuffers);
  _myMsg.sizeOfBuffers  (sizeofBuffers);

  _tmo.tv_sec  = 0;
  _tmo.tv_nsec = 0;

  sem_init(&_sem, 0, 1);

  _init();
}

XtcMonitorServer::~XtcMonitorServer() 
{ 
  delete _sequence;
  sem_destroy(&_sem);
  pthread_kill(_threadID, SIGTERM);
  printf("Not Unlinking Shared Memory... \n");

  printf("Unlinking Message Queues... \n");
  mq_close(_discoveryQueue);
  mq_close(_myInputEvQueue);
  for(unsigned i=0; i<_numberOfClients; i++) {
    mq_close(_myOutputEvQueue[i]);
    mq_close(_myOutputTrQueue[i]);
  }
  mq_close(_shuffleQueue);

  char qname[128];
  XtcMonitorMsg::discoveryQueue (_tag, qname);  mq_unlink(qname);
  for(unsigned i=0; i<_numberOfClients; i++) {
    XtcMonitorMsg::eventInputQueue     (_tag,i,qname); mq_unlink(qname);
    XtcMonitorMsg::transitionInputQueue(_tag,i,qname); mq_unlink(qname);
  }
  XtcMonitorMsg::eventInputQueue     (_tag,_numberOfClients,qname); mq_unlink(qname);
  sprintf(qname, "/PdsShuffleQueue_%s",_tag);  mq_unlink(qname);

  delete[] _postmarks;
}

void XtcMonitorServer::_claim(unsigned index)
{
  unsigned ibit =  (1<<index);
  if (!(_freelist & ibit)) {
    _freelist |= ibit;
    _nfree++;
  }
}

bool XtcMonitorServer::_claimOutputQueues(unsigned depth)
{
  for(unsigned j=0; j<_numberOfClients; j++) {
    while (mq_timedreceive(_myOutputEvQueue[_numberOfClients-1-j], 
                           (char*)&_myMsg, sizeof(_myMsg), NULL, &_tmo) != -1) {
      _claim(_myMsg.bufferIndex());
      if (_nfree >= depth)
        return true;
    }
  }
  return false;
}

bool XtcMonitorServer::_send_sequence()
{
  unsigned depth = _sequence->depth();

  if (_nfree < depth) {

    struct mq_attr attr;
    mq_getattr(_myInputEvQueue, &attr);

    unsigned curmsg = attr.mq_curmsgs;
    for (unsigned i=0; i<curmsg; i++) {
      if (mq_receive(_myInputEvQueue, (char*)&_myMsg, sizeof(_myMsg), NULL) < 0) 
        perror("mq_receive");
      _claim(_myMsg.bufferIndex());
    }
    
    timespec tv;
    clock_gettime(CLOCK_REALTIME,&tv);
    timespec tmo = tv; tmo.tv_sec -= TMO_SEC;

    if (_nfree < depth) {
      if (tmo.tv_sec > _postmarks[_lastSent].tv_sec) {
        if (!_claimOutputQueues(depth)) {
          for(unsigned i=0; i<_numberOfEvBuffers; i++) {
            if ((_freelist &(1<<i))==0 && tmo.tv_sec > _postmarks[i].tv_sec) {
              char buff[128];
              time_t t = _postmarks[i].tv_sec;
              strftime(buff,128,"%H:%M:%S",localtime(&t));
              printf("recover shmem buffer %d : %s.%09u\n",
                     i, buff, (unsigned)_postmarks[i].tv_nsec);
              _claim(i);
            }
          }
        }
      }
    }
  }

  if (_nfree < depth)
    return false;
  
  for(unsigned i=0; i<depth; i++) {

    unsigned j=0;
    while( (_freelist&(1<<j))==0 )
      j++;

    _myMsg.bufferIndex(j);
    _freelist ^= (1<<j);
    _nfree--;
        
    ShMsg m(_myMsg, _sequence->dgram(i));
    if (mq_timedsend(_shuffleQueue, (const char*)&m, sizeof(m), 0, &_tmo)) {
      printf("ShuffleQ timedout\n");
      _deleteDatagram(_sequence->dgram(i));
    }
  }
  _sequence->clear();
  return true;
}

XtcMonitorServer::Result XtcMonitorServer::events(Dgram* dg) 
{
  Dgram& dgrm = *dg;
  if (sizeof(dgrm)+dgrm.xtc.sizeofPayload() > _sizeOfBuffers) {
    printf("XtcMonitorServer skipping %s with payload size %d - too large\n",
	   TransitionId::name(dgrm.seq.service()), dgrm.xtc.sizeofPayload());
    //    return Handled;
    exit(1);
  }

  if (dgrm.seq.service() == TransitionId::L1Accept) {
    if (_sequence->complete())
      if (!_send_sequence())
        return Handled;

    _sequence->insert(dg);
    if (_sequence->complete())
      _send_sequence();
    return Deferred;
  }
  else {


    for(unsigned i=0; i<_sequence->current(); i++)
      _deleteDatagram(_sequence->dgram(i));
    _sequence->clear();

    if (_freeTr.empty()) {
      printf("No buffers available for transition !\n");
      abort();
    }

    int ibuffer = _freeTr.front(); _freeTr.pop();
    if (ibuffer<0 || ibuffer>=int(_numberOfEvBuffers+numberofTrBuffers)) {
      printf("XtcMonitorServer popped buffer %d\n",ibuffer);
      abort();
    }

    _myMsg.bufferIndex(ibuffer);
    _copyDatagram(dg, _myShm + _sizeOfBuffers*ibuffer);

    //
    //  Cache the transition for any clients which may not be listening
    //
    sem_wait(&_sem);

    TransitionId::Value trid = dgrm.seq.service();

    if ( _cachedTr.empty() ) {
      if (trid==TransitionId::Map) 
	_push_transition(ibuffer);
    }
    else {
      const Dgram& odg = *reinterpret_cast<const Dgram*>(_myShm + _sizeOfBuffers*_cachedTr.top());
      TransitionId::Value otrid = odg.seq.service();
      if (trid == otrid+2)
	_push_transition(ibuffer);
      else if (trid == otrid+1) {
	_pop_transition();
	_freeTr.push(ibuffer);
      }
      else {
	_freeTr.push(ibuffer);
      }
    }

    sem_post(&_sem);

    //
    //  Steal all event buffers from the clients
    //
    for(unsigned i=0; i<_numberOfClients; i++)
      _moveQueue(_myOutputEvQueue[i], _myInputEvQueue);

    //
    //  Broadcast the transition to all listening clients
    //
    for(unsigned i=0; i<_numberOfClients; i++) {
      if (mq_timedsend(_myOutputTrQueue[i], (const char*)&_myMsg, sizeof(_myMsg), 0, &_tmo))
        ;  // best effort
    }

  }
  return Handled;
}

void XtcMonitorServer::routine()
{
  while(1) {
    if (::poll(_pfd,2,-1) > 0) {
      if (_pfd[0].revents & POLLIN)
        _initialize_client();

      if (_pfd[1].revents & POLLIN) {
        ShMsg m;
        if (mq_receive(_shuffleQueue, (char*)&m, sizeof(m), NULL) < 0)
          perror("mq_receive");

        _copyDatagram(m.dg(),_myShm+_sizeOfBuffers*m.msg().bufferIndex());
        _deleteDatagram(m.dg());

	//
	//  Send this event to the first available client
	//
	for(unsigned i=0; i<=_numberOfClients; i++)
	  if (mq_timedsend(_myOutputEvQueue[i], (const char*)&m.msg(), sizeof(m.msg()), 0, &_tmo))
	    ; //	    printf("outputEv timedout to client %d\n",i);
	  else {
            clock_gettime(CLOCK_REALTIME,&_postmarks[m.msg().bufferIndex()]);
            _lastSent = m.msg().bufferIndex();
	    break;
          }
      }
    }
  }
}

static void* TaskRoutine(void* task)
    {
  XtcMonitorServer* srv = (XtcMonitorServer*)task;
  srv->routine();
  return srv;
    }

int XtcMonitorServer::_init() 
{ 
  const char* p = _tag;
  char* shmName    = new char[128];
  char* toQname    = new char[128];
  char* fromQname  = new char[128];

  sprintf(shmName  , "/PdsMonitorSharedMemory_%s",p);
  _pageSize = (unsigned)sysconf(_SC_PAGESIZE);

  int ret = 0;
  _sizeOfShm = (_numberOfEvBuffers + numberofTrBuffers) * _sizeOfBuffers;
  unsigned remainder = _sizeOfShm%_pageSize;
  if (remainder) _sizeOfShm += _pageSize - remainder;

  umask(1);  // try to enable world members to open these devices.

  int shm = shm_open(shmName, OFLAGS, PERMS);
  if (shm < 0) {ret++; perror("shm_open");}

  if ((ftruncate(shm, _sizeOfShm))<0) {ret++; perror("ftruncate");}

  _myShm = (char*)mmap(NULL, _sizeOfShm, PROT_READ|PROT_WRITE, MAP_SHARED, shm, 0);
  if (_myShm == MAP_FAILED) {ret++; perror("mmap");}

  mq_attr q_attr;
  q_attr.mq_maxmsg  = _numberOfEvBuffers;
  q_attr.mq_msgsize = (long int)sizeof(XtcMonitorMsg);
  q_attr.mq_flags   = O_NONBLOCK;

  XtcMonitorMsg::eventOutputQueue(p,_numberOfClients-1,toQname);
  _flushQueue(_myInputEvQueue  = _openQueue(toQname,q_attr));

  q_attr.mq_maxmsg  = _numberOfEvBuffers / _numberOfClients;
  q_attr.mq_msgsize = (long int)sizeof(XtcMonitorMsg);
  q_attr.mq_flags   = O_NONBLOCK;

  _myOutputEvQueue = new mqd_t[_numberOfClients];
  for(unsigned i=0; i<_numberOfClients; i++) {
    XtcMonitorMsg::eventInputQueue(p,i,toQname);
    _flushQueue(_myOutputEvQueue[i] = _openQueue(toQname,q_attr));
  }
  
  q_attr.mq_maxmsg  = _numberOfClients;
  q_attr.mq_msgsize = (long int)sizeof(XtcMonitorMsg);
  q_attr.mq_flags   = O_NONBLOCK;

  XtcMonitorMsg::discoveryQueue(p, fromQname);
  sprintf(fromQname, "/PdsFromMonitorDiscovery_%s",p);
  _pfd[0].fd      = _discoveryQueue  = _openQueue(fromQname,q_attr);
  _pfd[0].events  = POLLIN;
  _pfd[0].revents = 0;

  
  q_attr.mq_maxmsg  = numberofTrBuffers;
  q_attr.mq_msgsize = (long int)sizeof(XtcMonitorMsg);
  q_attr.mq_flags   = O_NONBLOCK;

  _myOutputTrQueue = new mqd_t[_numberOfClients];
  for(unsigned i=0; i<_numberOfClients; i++) {
    XtcMonitorMsg::transitionInputQueue(p,i,toQname);
    _flushQueue(_myOutputTrQueue[i] = _openQueue(toQname,q_attr));
  }

  q_attr.mq_maxmsg  = _numberOfEvBuffers;
  q_attr.mq_msgsize = (long int)sizeof(ShMsg);
  q_attr.mq_flags   = O_NONBLOCK;

  sprintf(toQname, "/PdsShuffleQueue_%s",p);
  _shuffleQueue = _openQueue(toQname, q_attr);
  { ShMsg m; _flushQueue(_shuffleQueue,(char*)&m, sizeof(m)); }

  _pfd[1].fd = _shuffleQueue;
  _pfd[1].events  = POLLIN;
  _pfd[1].revents = 0;

  // create the listening thread
  pthread_create(&_threadID,NULL,TaskRoutine,this);

  // prestuff the input queue which doubles as the free list
  _postmarks = new timespec[_numberOfEvBuffers];
  _freelist  = (1<<_numberOfEvBuffers)-1;
  _nfree     = _numberOfEvBuffers;
  _lastSent  = 0;

  for(int i=0; i<numberofTrBuffers; i++)
    _freeTr.push(i+_numberOfEvBuffers);

  delete[] shmName;
  delete[] toQname;
  delete[] fromQname;

  if (_sequence->depth() > _numberOfEvBuffers) {
    printf("Requested sequence length (%d) > number of buffers (%d)\n",
	   _sequence->depth(),_numberOfEvBuffers);
    ret++;
  }

  return ret;
}

void XtcMonitorServer::_initialize_client()
{
  sem_wait(&_sem);

  XtcMonitorMsg msg;
  if (mq_receive(_discoveryQueue, (char*)&msg, sizeof(msg), NULL) < 0) 
    perror("mq_receive");

  unsigned iclient = msg.bufferIndex();
  printf("_initialize_client %d\n",iclient);

  std::stack<int> tr;
  while(!_cachedTr.empty()) {
    tr.push(_cachedTr.top());
    _cachedTr.pop();
  }
  while(!tr.empty()) {
    int ibuffer = tr.top(); tr.pop();
    _myMsg.bufferIndex(ibuffer);

    //     { Dgram& dgrm = *reinterpret_cast<Dgram*>(_myShm + _sizeOfBuffers * _myMsg.bufferIndex());
    //       printf("Sending tr %s to mq %d\nmsg %x/%x/%x\n",
    // 	     TransitionId::name(dgrm.seq.service()),
    // 	     _myOutputTrQueue[iclient],
    // 	     _myMsg.bufferIndex(),
    // 	     _myMsg.numberOfBuffers(),
    // 	     _myMsg.sizeOfBuffers()); }

    if (mq_send(_myOutputTrQueue[iclient], (const char*)&_myMsg, sizeof(_myMsg), 0)) 
      ;   // best effort only
    _cachedTr.push(ibuffer);
  }

  sem_post(&_sem);
}

void XtcMonitorServer::_copyDatagram(Dgram* p, char* b) 
{
  Dgram* dg = (Dgram*)p;
  memcpy((char*)b, dg, sizeof(Dgram)+dg->xtc.sizeofPayload());
}

void XtcMonitorServer::_deleteDatagram(Dgram* p) {}

mqd_t XtcMonitorServer::_openQueue(const char* name, mq_attr& attr) 
{
  mqd_t q = mq_open(name,  O_CREAT|O_RDWR, PERMS, &attr);
  if (q == (mqd_t)-1) {
    perror("mq_open output");
    printf("mq_attr:\n\tmq_flags 0x%0lx\n\tmq_maxmsg 0x%0lx\n\tmq_msgsize 0x%0lx\n\t mq_curmsgs 0x%0lx\n",
        attr.mq_flags, attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs );
    fprintf(stderr, "Initializing XTC monitor server encountered an error!\n");
    delete this;
    exit(EXIT_FAILURE);
  }
  else {  // Open twice to set all of the attributes
    printf("Opened queue %s (%d)\n",name,q);
  }

  mq_attr r_attr;
  mq_getattr(q,&r_attr);
  if (r_attr.mq_maxmsg != attr.mq_maxmsg ||
      r_attr.mq_msgsize!= attr.mq_msgsize) {

    printf("Failed to set queue attributes the first time.\n");
    mq_close(q);

    mqd_t q = mq_open(name,  O_CREAT|O_RDWR, PERMS, &attr);
    mq_getattr(q,&r_attr);

    if (r_attr.mq_maxmsg != attr.mq_maxmsg ||
	r_attr.mq_msgsize!= attr.mq_msgsize) {
      printf("Failed to set queue attributes the second time.\n");
      printf("open attr  %lx %lx %lx  read attr %lx %lx %lx\n",
	     attr.mq_flags, attr.mq_maxmsg, attr.mq_msgsize,
	     r_attr.mq_flags, r_attr.mq_maxmsg, r_attr.mq_msgsize);
    }
  }

  return q;
}

void XtcMonitorServer::_flushQueue(mqd_t q)
{
  XtcMonitorMsg m; 
  _flushQueue(q,(char*)&m,sizeof(m)); 
}

void XtcMonitorServer::_flushQueue(mqd_t q, char* m, unsigned sz) 
{
  // flush the queues just to be sure they are empty.
  struct mq_attr attr;
  do {
    mq_getattr(q, &attr);
    if (attr.mq_curmsgs)
      mq_timedreceive(q, m, sz, NULL, &_tmo);
  } while (attr.mq_curmsgs);
}

void XtcMonitorServer::_moveQueue(mqd_t iq, mqd_t oq) 
{
  XtcMonitorMsg m;
  struct mq_attr attr;
  do {
    mq_getattr(iq, &attr);
    if (attr.mq_curmsgs) {
      if (mq_timedreceive(iq, (char*)&m, sizeof(m), NULL, &_tmo) == -1)
        perror("moveQueue: mq_timedreceive");
      else if (mq_send   (oq, (char*)&m, sizeof(m), 0) == -1) {
        printf("Failed to reclaim buffer %i : %s\n",
	       m.bufferIndex(), strerror(errno));
      }
    }
  } while (attr.mq_curmsgs);
}

void XtcMonitorServer::_push_transition(int ibuffer)
{
  _cachedTr.push(ibuffer);
}

void XtcMonitorServer::_pop_transition()
{
  _freeTr.push(_cachedTr.top());
  _cachedTr.pop();
}

