#ifndef Pds_XtcPool_hh
#define Pds_XtcPool_hh

//#define DUMP_DMGOFF

namespace Pds {
  class Dgram;
  namespace Ana {
    extern bool _live;
  
    class XtcPool {
    public:
      XtcPool (unsigned nevents, unsigned eventsize);
      ~XtcPool();
  
      // Read a complete event into the pool from file descriptor 'fd'.
      // Upon failure, insert 0 into the pool and return false.
      // For 'live' file read, always return true.
      bool push(int fd);   
      Pds::Dgram* pop(Pds::Dgram* r);
  
      // Release the semaphore to unblock all waiting threads
      void unblock();
  
    private:
      unsigned _eventsize;
      class SafeBufferQueue* _pend;
      class SafeBufferQueue* _free;
      void _waitAndFill(int fd, char* p, unsigned sz);
    };
  }
}

#endif
