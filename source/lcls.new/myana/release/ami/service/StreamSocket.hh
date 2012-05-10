#ifndef Ami_StreamSocket_HH
#define Ami_StreamSocket_HH

#include "ami/service/Socket.hh"

namespace Ami {

  class Ins;
  class Message;

  class StreamSocket : public Socket {
  public:
    StreamSocket();
    StreamSocket(int socket);
    virtual ~StreamSocket();
    
    int fetch  (Message*);
    int readv  (const iovec* iov, int iovcnt);
    int connect(const Ins& dst);
    int listen (const Ins& src);
    int accept();
  };
};

#endif
