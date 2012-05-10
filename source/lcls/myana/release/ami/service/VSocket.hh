#ifndef Ami_VSocket_HH
#define Ami_VSocket_HH

#include "ami/service/Socket.hh"

#include <sys/types.h>

namespace Ami {

  class Message;

  class VSocket : public Socket {
  public:
    VSocket();
    ~VSocket();
    
    virtual int readv(const iovec* iov, int iovcnt);

    int peek (Message*);
    int flush();

  protected:
    struct msghdr _rhdr;
  private:
    iovec*        _iovs;
    int           _iovcnt;
    bool          _peeked;
  };
};

#endif
