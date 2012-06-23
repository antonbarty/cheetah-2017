#ifndef Ami_Loopback_HH
#define Ami_Loopback_HH

#include "ami/service/Socket.hh"

namespace Ami {

  class Ins;

  class Loopback : public Socket {
  public:
    Loopback();
    ~Loopback();
    virtual int readv(const iovec* iov, int iovcnt);
  };
};

#endif
