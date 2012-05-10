#ifndef AMI_SOCKADDR
#define AMI_SOCKADDR

#include "ami/service/Ins.hh"

#include <sys/socket.h>
#include <string.h> // for memset()

namespace Ami {
class Sockaddr {
public:
  Sockaddr() {
    _sockaddr.sin_family      = AF_INET;
  }
  
  Sockaddr(const Ins& ins) {
    _sockaddr.sin_family      = AF_INET;
    _sockaddr.sin_addr.s_addr = htonl(ins.address());
    _sockaddr.sin_port        = htons(ins.portId()); 
    memset(_sockaddr.sin_zero,0,sizeof(_sockaddr.sin_zero));
  }
  
  void get(const Ins& ins) {
    _sockaddr.sin_addr.s_addr = htonl(ins.address());
    _sockaddr.sin_port        = htons(ins.portId());     
  }

  Ins get() const {
    return Ins(ntohl(_sockaddr.sin_addr.s_addr),
	       ntohs(_sockaddr.sin_port));
  }

  sockaddr* name() const {return (sockaddr*)&_sockaddr;}
  inline int sizeofName() const {return sizeof(_sockaddr);}

  bool operator==(const Sockaddr& s) const { return _sockaddr.sin_addr.s_addr==s._sockaddr.sin_addr.s_addr && _sockaddr.sin_port==s._sockaddr.sin_port; }
      
private:
  sockaddr_in _sockaddr;
};
}
#endif
