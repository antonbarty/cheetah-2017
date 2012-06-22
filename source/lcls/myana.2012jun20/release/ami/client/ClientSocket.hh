#ifndef Ami_ClientSocket_HH
#define Ami_ClientSocket_HH

#include "ami/service/Fd.hh"
#include "ami/service/TSocket.hh"

namespace Ami {

  class ClientManager;

  class ClientSocket : public Fd,
		       public TSocket {
  public:
    ClientSocket(ClientManager&);
    ClientSocket(ClientManager&, int);
    ~ClientSocket();
  public:
    int fd() const;
    int processIo();
    int processIo(const char*,int);
  private:
    ClientManager& _mgr;
  };
};

#endif
