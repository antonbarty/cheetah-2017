#ifndef Ami_BSocket_HH
#define Ami_BSocket_HH

#include "ami/service/Socket.hh"

#include <sys/types.h>

namespace Ami {

  class Message;

  class BSocket : public Socket {
  public:
    BSocket(unsigned size);
    ~BSocket();
  public:    
    virtual int readv(const iovec* iov, int iovcnt);
  public:
    char* data();
  private:
    unsigned _size;
    char*    _buffer;
  };
};

#endif
