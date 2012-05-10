#ifndef PDS_MONITOR_HH
#define PDS_MONITOR_HH

#include <pthread.h>

namespace Ami {
  class Monitor {
  public:
    Monitor(const char* name, const bool debug = false);
    ~Monitor();
    void lock();
    void unlock();
  private:
    const char* _name;
    const bool _debug;
    pthread_t _owner;
    pthread_cond_t _condition;
    pthread_mutex_t _mutex;
  };
}

#endif
