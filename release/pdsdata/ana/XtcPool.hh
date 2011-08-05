#ifndef Pds_XtcPool_hh
#define Pds_XtcPool_hh

#include <queue>
#include <semaphore.h>

//#define DUMP_DMGOFF

namespace Pds
{  
  
class Dgram;

namespace Ana
{  
extern bool _live;
  
class XtcPool {
public:
  XtcPool (unsigned nevents, unsigned eventsize);
  ~XtcPool();
  
  //
  //  Read a complete event into the pool from file descriptor 'fd'.
  //  Upon failure, insert 0 into the pool and return false
  //  For 'live' file read, always return true.
  bool        push(int fd);   
  Pds::Dgram* pop (Pds::Dgram* r);
  
  /*
   * release the semeaphore to unblock all waiting threads
   */
  void unblock();
  
private:
  std::queue<char*> _pend;
  std::queue<char*> _free;
  sem_t _pend_sem;
  sem_t _free_sem;
  unsigned _eventsize;
  bool     _bStop;
};

} // namespace Ana
} // namespace Pds

#endif // #ifndef Pds_XtcPool_hh
