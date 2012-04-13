#include "Lock.hh"
#ifdef VXWORKS
#include "taskLib.h"
#endif

using namespace Ami;

#ifdef VXWORKS
Lock::Lock(unsigned retries) : _lock(Free), _retries(retries) {}
#else
Lock::Lock(unsigned retries) : _lock(Semaphore::FULL),
  _retries(retries) {}
#endif

#define _asm asm volatile

unsigned Lock::tryOnce() {
#ifdef VXWORKS
  unsigned error;
  /* asm documentation we have is unclear */
  /* make all asm statements volatile until we really understand it */

  unsigned oldlockword,newlockword,currentlockbit,sdfg;
  /* load old lock word with reservation, volatile since may have changed */
  _asm ("lwarx %0,0,%1" : "=r" (oldlockword) : "b" (&_lock));
  /* extract current lock bit. */
  _asm ("andi. %0,%1,1" : "=r" (currentlockbit) : "r" (oldlockword));
  /* set lockbit (in parallel with prev instr) */
  _asm ("ori %0,%1,1" : "=r" (newlockword) : "r" (oldlockword));
  _asm ("bne- 1f         /* if already in use, goto error */
                         /* minus indicates unlikely to branch */
         stwcx. %1,0,%2  /* try to store our lock */
         li %0,0         /* try to overlap this with stwcx instruction above */
         beq+ 2f         /* if we succeeded, go to success */
                         /* plus indicates branch likely to be taken */

         1:              /* error.  set error flag */
         li %0,1

         2:"             /* success */

         : "=r" (error) : "r" (newlockword), "b" (&_lock) : "memory");
  _asm("isync");        /* make sure lock is complete before we touch */
                        /* anything important */
  return error;
#else
  _lock.take();
  return 0;
#endif
}

void Lock::get() {
  unsigned value;
  if ((value = tryOnce())) {
    unsigned count = _retries;
    do {
#ifdef VXWORKS
      // a hack for the moment, to allow somebody else to run and release the
      // lock
      taskDelay(1);
#endif
      if ((value = tryOnce()) == 0) return;
    } while (--count);
    cantLock();
  }
}

void Lock::release() {
#ifdef VXWORKS
  unsigned temp;
  /* put in sync cuz ric has case where he needs multiprocessor locks */
  asm("sync");
  _lock = 0x0;
//   asm("li %0,0" : "=r" (temp));
//   asm("stwu %1,0(%0)" : "=r" (_lock) : "r" (temp) );
#else
  _lock.give();
#endif
}
