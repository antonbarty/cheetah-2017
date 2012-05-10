#ifndef Ami_VClientSocket_HH
#define Ami_VClientSocket_HH

#include "ami/service/VSocket.hh"
#include "ami/service/Sockaddr.hh"

namespace Ami {

  class Ins;

  class VClientSocket : public VSocket {
  public:
    VClientSocket();
    ~VClientSocket();
    
    int set_dst(const Ins&,
		int interface);

    const Sockaddr& peer() const { return _peer; }
  private:
    Sockaddr      _addr;
    Sockaddr      _peer;
  };
};

#endif
