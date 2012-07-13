#ifndef Ami_TSocket_HH
#define Ami_TSocket_HH

#include "ami/service/Socket.hh"
#include "ami/service/Exception.hh"
#include "ami/service/Ins.hh"

#include <sys/types.h>

namespace Ami {

  class Ins;
  class Message;

  class TSocket : public Socket {
  public:
    TSocket() throw(Event);
    TSocket(int) throw(Event);

    ~TSocket();

  public:
    void bind(const Ins&) throw(Event);
    void connect(const Ins&) throw(Event);
    Ins  ins() const throw(Event);
 
    virtual int readv(const iovec* iov, int iovcnt);

  protected:
    struct msghdr _rhdr;
  };
};

#endif
