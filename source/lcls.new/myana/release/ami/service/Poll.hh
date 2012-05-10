#ifndef Pds_Poll_hh
#define Pds_Poll_hh

#include "ami/service/Routine.hh"
#include "ami/service/Semaphore.hh"

#include <poll.h>

class iovec;

namespace Ami {

  class Fd;
  class Task;
  class Socket;

  class Poll : public Routine {
  public:
    Poll(int timeout);
    ~Poll();
  public:
    void start();
    void stop ();
    void routine();
  public:
    int  nfds() const;
    Fd&  fds(int) const;
    void manage  (Fd&);
    virtual void unmanage(Fd&);
    void bcast_in (const char*,int);
    void bcast_out(const char*,int);
    void bcast_out(const iovec*,int);
    void post     (const char*,int);
  private:
    void bcast   (const char*,int,int);
    void bcast   (const iovec*,int,int);
    int poll();
    void adjust ();
  protected:
    int  timeout() const;
    void timeout(int);
  private:
    virtual int processTmo();
    virtual int processIn (const char*,int);
  private:
    int        _timeout;
    Task*      _task;
    Socket*    _loopback;
    int        _nfds;
    int        _maxfds;
    Fd**       _ofd;
    pollfd*    _pfd;
    Semaphore  _sem;
    char*      _buffer;
  };

};

#endif
