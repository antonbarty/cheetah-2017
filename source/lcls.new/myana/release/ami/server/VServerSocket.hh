#ifndef Ami_VServerSocket_HH
#define Ami_VServerSocket_HH

#include "ami/service/VSocket.hh"

#include "ami/service/Exception.hh"
#include "ami/service/Ins.hh"
#include "ami/service/Sockaddr.hh"

namespace Ami {

  class VServerSocket : public VSocket {
  public:
    VServerSocket(const Ins& mcast,
		  int interface,
		  Sockaddr peer = Ins()) throw(Event);
    ~VServerSocket();
  public:
    const Sockaddr& peer() const { return _peer; }
    const Ins&      mcast() const { return _mcast; }
  private:
    Sockaddr _peer;
    Ins      _mcast;
    int      _interface;
  };
};

#endif
