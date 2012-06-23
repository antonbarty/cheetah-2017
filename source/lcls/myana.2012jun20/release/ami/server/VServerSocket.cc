#include "VServerSocket.hh"

#include <stdio.h>
#include <errno.h>
#include <string.h>

using namespace Ami;

VServerSocket::VServerSocket(const Ins& mcast,
			     int        interface,
			     Sockaddr   peer) throw(Event) :
  _peer     (peer),
  _mcast    (mcast),
  _interface(interface)
{
  int sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd<0) {
    throw Event("VServerSocket failed to open socket",strerror(errno));
  }

  int y=1;
  if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&y, sizeof(y)) == -1) {
    throw Event("VServerSocket set broadcast error",strerror(errno));
  }

  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&y, sizeof(y)) == -1) {
    throw Event("VServerSocket set reuseaddr error",strerror(errno));
  }


  Sockaddr sa(mcast);
  int result = ::bind(sockfd, (sockaddr*)sa.name(), sa.sizeofName());
  if (result < 0) {
    throw Event("VServerSocket bind error",strerror(errno));
  }

  sockaddr_in name;
  socklen_t name_len=sizeof(name);
  getsockname(sockfd,(sockaddr*)&name,&name_len);
//   printf("VServerSocket %d bound to %x/%d\n",
// 	 sockfd, ntohl(name.sin_addr.s_addr), ntohs(name.sin_port));

  if (Ins::is_multicast(mcast)) {
    struct ip_mreq ipMreq;
    bzero ((char*)&ipMreq, sizeof(ipMreq));
    ipMreq.imr_multiaddr.s_addr = htonl(mcast.address());
    ipMreq.imr_interface.s_addr = htonl(interface);
    int error_join = setsockopt (sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
				 (char*)&ipMreq, sizeof(ipMreq));
    if (error_join==-1) {
      throw Event("VServerSocket failed to join mcast group",strerror(errno));
    }
  }

  _socket = sockfd;

  _hdr.msg_name    = &_peer;
  _hdr.msg_namelen = sizeof(_peer);
  _rhdr.msg_name    = &_peer;
  _rhdr.msg_namelen = sizeof(_peer);
}

VServerSocket::~VServerSocket()
{
  struct ip_mreq ipMreq;
  bzero ((char*)&ipMreq, sizeof(ipMreq));
  ipMreq.imr_multiaddr.s_addr = htonl(_mcast.address());
  ipMreq.imr_interface.s_addr = htonl(_interface);
  int error_resign = setsockopt (_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&ipMreq,
				 sizeof(ipMreq));
  if (error_resign)
    printf("~VServerSocket error resigning from group %x : reason %s\n",
	   _mcast.address(), strerror(errno));
}

