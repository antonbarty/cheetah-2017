#include "pdsdata/ana/XtcPool.hh"
#include "pdsdata/xtc/Dgram.hh"

using std::queue;

namespace Pds
{  
namespace Ana
{
  
XtcPool::XtcPool(unsigned nevents, unsigned eventsize) :
  _eventsize(eventsize), _bStop(false)
{
  sem_init(&_pend_sem, 0, 0);
  sem_init(&_free_sem, 0, nevents);
  while(nevents--) {
    char* b = new char[eventsize];
    _free.push(b);
  }
}

XtcPool::~XtcPool()
{
  sem_destroy(&_pend_sem);
  sem_destroy(&_free_sem);
  while(!_pend.empty()) {
    char* b = _pend.front();
    if (b) delete b;
    _pend.pop();
  }
  while(!_free.empty()) {
    delete _free.front();
    _free.pop();
  }
}

//
//  Read a complete event into the pool from file descriptor 'fd'.
//  Upon failure, insert 0 into the pool and return false
//  For 'live' file read, always return true.
//
bool XtcPool::push(int fd)
{
  while (sem_wait(&_free_sem)); // keep running sem_wait(), if it is interrupted by signal
  if (_bStop)
    return false;
    
  char* b = _free.front();
  _free.pop();
  
  //off64_t i64Offset = ::lseek64(fd,0,SEEK_CUR); //!!debug
  
  unsigned sz = sizeof(Pds::Dgram);
  ssize_t rsz = ::read(fd, b, sz);
  
  if (rsz == ssize_t(sz)) {
    Pds::Dgram* dg = reinterpret_cast<Pds::Dgram*>(b);
    sz = dg->xtc.sizeofPayload();
    
    //{//!!debug
    //  char sTimeBuff[128], sDateTimeBuff[128];
    //  time_t t = dg->seq.clock().seconds();
    //  strftime(sTimeBuff,128,"%T",localtime(&t));
    //  strftime(sDateTimeBuff,128,"%Z %a %F %T",localtime(&t));
    //  
    //  printf( "\npush(): dg(%p) %s ctl 0x%x vec %d fid 0x%x %s.%03u "
    //   "offset 0x%Lx env 0x%x damage 0x%x extent 0x%x\n",
    //   dg,
    //   Pds::TransitionId::name(dg->seq.service()), dg->seq.stamp().control(),
    //   dg->seq.stamp().vector(), dg->seq.stamp().fiducials(), 
    //   sDateTimeBuff, (int) (dg->seq.clock().nanoseconds() / 1e6),
    //   i64Offset, dg->env.value(), dg->xtc.damage.value(), dg->xtc.extent);           
    //}
    
#ifdef DUMP_DMGOFF
    if (dg->xtc.damage.value()&(1<<Pds::Damage::DroppedContribution)) {
      off64_t pos = ::lseek64(fd,0,SEEK_CUR);
      char buff[128];
      time_t t = dg->seq.clock().seconds();
      strftime(buff,128,"%H:%M:%S",localtime(&t));
      printf("%s.%09u dmg %08x @ %llu\n",
             buff,dg->seq.clock().nanoseconds(),dg->xtc.damage.value(),pos);
    }
#endif
    
    if (sz + sizeof(Pds::Dgram) > _eventsize) {
      printf("Event size (%d) is greater than pool size(%d)\n",
             sz, _eventsize);
      printf("Skipping remainder of file.\n");
    }
    else {
      rsz = ::read(fd, dg->xtc.payload(), sz);
      if (rsz == ssize_t(sz)) {
        _pend.push(b);
        sem_post(&_pend_sem);
        return true;
      }
    }
  }

  //printf("push failed: read dgram (size %d) returned %d\n", sz, rsz);//!!debug
  
  _free.push(b);
  sem_post(&_free_sem);
  
  _pend.push(0);
  sem_post(&_pend_sem);

  if (_live) {
    printf("\rLive read waits...");
    fflush(stdout);
    sleep(1);
    return true;
  }
  else
    return false;
}

Pds::Dgram* XtcPool::pop(Pds::Dgram* r)
{    
  //printf("pop() start r = %p\n", r);//!!debug
  
  if (r) {
    _free.push(reinterpret_cast<char*>(r));
    sem_post(&_free_sem);
  }

  while (sem_wait(&_pend_sem)); // keep running sem_wait(), if it is interrupted by signal
  if (_bStop)
    return NULL;
    
  char* b = _pend.front();
  _pend.pop();

  //if ( b != NULL ) //!!debug
  //{
  //  Pds::Dgram* dg = reinterpret_cast<Pds::Dgram*>(b);
  //  char sTimeBuff[128], sDateTimeBuff[128];
  //  time_t t = dg->seq.clock().seconds();
  //  strftime(sTimeBuff,128,"%T",localtime(&t));
  //  strftime(sDateTimeBuff,128,"%Z %a %F %T",localtime(&t));
  //  
  //  printf( "\npop(): dg(%p) %s ctl 0x%x vec %d fid 0x%x %s.%03u "
  //   "env 0x%x damage 0x%x extent 0x%x\n",
  //   dg,
  //   Pds::TransitionId::name(dg->seq.service()), dg->seq.stamp().control(),
  //   dg->seq.stamp().vector(), dg->seq.stamp().fiducials(), 
  //   sDateTimeBuff, (int) (dg->seq.clock().nanoseconds() / 1e6),
  //   dg->env.value(), dg->xtc.damage.value(), dg->xtc.extent);           
  //}
  
  return reinterpret_cast<Pds::Dgram*>(b);
}

/*
 * release the semeaphore to unblock all waiting threads
 */
void XtcPool::unblock()
{
  _bStop = true;
  sem_post(&_free_sem);
  sem_post(&_pend_sem);    
}  

  
} // namespace Ana
} // namespace Pds

