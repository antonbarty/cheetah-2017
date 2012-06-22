#include "pdsdata/ana/XtcPool.hh"
#include "pdsdata/xtc/Dgram.hh"

#include <queue>
#include <semaphore.h>

namespace Pds {
  namespace Ana {

    // Copied from pds/service/SafeQueue.hh and specialized as <char *, true>
    class SafeBufferQueue {
    private:
      const char* _name;
      bool _stop;
      std::queue<char*> _queue;
      pthread_cond_t _condition;
      pthread_mutex_t _mutex;

    public:
      SafeBufferQueue(const char* name) :
        _name(name),
        _stop(false) {
        pthread_cond_init(&_condition, NULL);
        pthread_mutex_init(&_mutex, NULL);
      }

      ~SafeBufferQueue() {
        pthread_mutex_lock(&_mutex);
        _stop = true;
        while(!_queue.empty()) {
          char* item = _queue.front();
          if (item != NULL) {
            delete[] item;
          }
          _queue.pop();
        }
        pthread_cond_broadcast(&_condition);
        pthread_mutex_unlock(&_mutex);
      }

      void unblock() {
        pthread_mutex_lock(&_mutex);
        _stop = true;
        pthread_cond_broadcast(&_condition);
        pthread_mutex_unlock(&_mutex);
      }  

      void push(char* item) {
        //printf("~~~~ PUSH %s %p\n", _name, item);
        pthread_mutex_lock(&_mutex);
        _queue.push(item);
        pthread_cond_signal(&_condition);
        pthread_mutex_unlock(&_mutex);
      }

      char* pop() {
        pthread_mutex_lock(&_mutex);
        for (;;) {
          if (_stop) {
            pthread_cond_signal(&_condition);
            pthread_mutex_unlock(&_mutex);
            return NULL;
          }
          if (! _queue.empty()) {
            break;
          }
          pthread_cond_wait(&_condition, &_mutex);
        }
        char* item = _queue.front();
        _queue.pop();
        pthread_mutex_unlock(&_mutex);
        //printf("~~~~ POP  %s %p\n", _name, item);
        return item;
      }
    };

    XtcPool::XtcPool(unsigned nevents, unsigned eventsize) :
      _eventsize(eventsize),
      _pend(new SafeBufferQueue("pend")),
      _free(new SafeBufferQueue("free"))
    {
      while(nevents--) {
        char* b = new char[eventsize];
        _free->push(b);
      }
    }

    XtcPool::~XtcPool() {
      delete _pend;
      delete _free;
    }

    //
    //  Read a complete event into the pool from file descriptor 'fd'.
    //  Upon failure, insert 0 into the pool and return false
    //  For 'live' file read, always return true.
    //
    bool XtcPool::push(int fd) {
      char* b = _free->pop();
      if (b == NULL) {
        return false;
      }
  
      unsigned sz = sizeof(Pds::Dgram);
      ssize_t rsz = ::read(fd, b, sz);
      if (rsz != ssize_t(sz) && _live && rsz > 0) {
        _waitAndFill(fd, b+rsz, sz-rsz);
        rsz = ssize_t(sz);
      }

      if (rsz == ssize_t(sz)) {
        Pds::Dgram* dg = reinterpret_cast<Pds::Dgram*>(b);
        sz = dg->xtc.sizeofPayload();
    
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
          printf("Event size (%d) is greater than pool size(%d); skipping remainder of file\n", sz, _eventsize);
        } else {
          rsz = ::read(fd, dg->xtc.payload(), sz);
          if (rsz == ssize_t(sz)) {
            _pend->push(b);
            return true;
          } else if (_live) {
            if (rsz==-1) {
              perror("Error reading file");
              exit(1);
            }
            _waitAndFill(fd, dg->xtc.payload()+rsz, sz-rsz);
          }
        }
      }

      _free->push(b);
      _pend->push(0);

      if (_live) {
        printf("\rLive read waits...");
        fflush(stdout);
        sleep(1);
        return true;
      }

      return false;
    }

    Pds::Dgram* XtcPool::pop(Pds::Dgram* r) {
      if (r != NULL) {
        _free->push(reinterpret_cast<char*>(r));
      }
      return reinterpret_cast<Pds::Dgram*>(_pend->pop());
    }

    void XtcPool::unblock() {
      _free->unblock();
      _pend->unblock();
    }  

    void XtcPool::_waitAndFill(int fd, char* p, unsigned sz) {
      do {
        printf("\rLive read waits... (%07d)",sz);
        fflush(stdout);
        sleep(1);
        ssize_t rsz = ::read(fd, p, sz);
        p  += rsz;
        sz -= rsz;
      } while (sz);
    }
  }
}
