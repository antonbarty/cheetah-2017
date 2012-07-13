#ifndef Pds_XtcMonitorServer_hh
#define Pds_XtcMonitorServer_hh

#include "pdsdata/app/XtcMonitorMsg.hh"

#include <pthread.h>
#include <semaphore.h>
#include <mqueue.h>
#include <queue>
#include <stack>
#include <poll.h>
#include <time.h>

namespace Pds {

  class Dgram;
  class EventSequence;

  class XtcMonitorServer {
  public:
    enum { numberofTrBuffers=8 };
  public:
    XtcMonitorServer(const char* tag,
		     unsigned sizeofBuffers, 
		     unsigned numberofEvBuffers, 
		     unsigned numberofClients,
		     unsigned sequenceLength=1);
    virtual ~XtcMonitorServer();
  public:
    enum Result { Handled, Deferred };
    Result events   (Dgram* dg);
    void routine    ();
  private:
    int  _init             ();
    void _initialize_client();
    mqd_t _openQueue       (const char* name, mq_attr&);
    void _flushQueue       (mqd_t q);
    void _flushQueue       (mqd_t q, char* m, unsigned sz);
    void _moveQueue        (mqd_t iq, mqd_t oq);
    void _push_transition  (int ibuffer);
    bool _send_sequence    ();
    void _claim            (unsigned);
    bool _claimOutputQueues(unsigned);
  protected:
    void _pop_transition   ();
  private:
    virtual void _copyDatagram  (Dgram* dg, char*);
    virtual void _deleteDatagram(Dgram* dg);
  private:
    const char*     _tag;
    unsigned        _sizeOfBuffers;
    unsigned        _numberOfEvBuffers;
    unsigned        _numberOfClients;
    unsigned        _sizeOfShm;
    char*           _bufferP;   //  pointer to the shared memory area being used
    char*           _myShm;     // the pointer to start of shared memory
    unsigned        _pageSize;
    XtcMonitorMsg   _myMsg;
    mqd_t           _myInputEvQueue;
    mqd_t*          _myOutputEvQueue; // nclients
    mqd_t*          _myOutputTrQueue; // nclients
    mqd_t           _discoveryQueue;
    std::stack<int> _cachedTr;
    std::queue<int> _freeTr;
    pollfd          _pfd[2];
    sem_t           _sem;
    mqd_t           _shuffleQueue;
    timespec        _tmo;
    pthread_t       _threadID;
    EventSequence*  _sequence;
    timespec*       _postmarks;
    unsigned        _freelist;
    unsigned        _nfree;
    unsigned        _lastSent;
  };
};

#endif
