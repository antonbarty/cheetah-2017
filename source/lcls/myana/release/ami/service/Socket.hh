#ifndef Ami_SOCKET_HH
#define Ami_SOCKET_HH

#include <sys/types.h>
#include <sys/socket.h>

//#define DBUG

class iovec;

namespace Ami {

  class Ins;

  class Socket {
  public:
    Socket(int socket=-1);
    virtual ~Socket();
    
    int socket() const;
    
    virtual int readv(const iovec* iov, int iovcnt) = 0;

    int read(void* buffer, int size);

    int write (const void* data, int size);
    int writev(const iovec* iov, int iovcnt);
    
    int setsndbuf(unsigned size);
    int setrcvbuf(unsigned size);
    
    int getsndbuf();
    int getrcvbuf();
    
    int close();
    
  protected:
    int _socket;
    struct msghdr _hdr;
  };
};

#endif
