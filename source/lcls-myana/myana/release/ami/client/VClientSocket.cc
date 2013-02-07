#include "VClientSocket.hh"

#include "ami/service/Ins.hh"

#include <stdio.h>
#include <errno.h>
#include <string.h>

using namespace Ami;

VClientSocket::VClientSocket() :
  _addr(Ins()),
  _peer(Ins())
{
  int sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd<0)
    printf("VClientSocket failed to allocate socket\n");

  _socket = sockfd;
  _hdr.msg_name    = _addr.name();
  _hdr.msg_namelen = _addr.sizeofName();
  _rhdr.msg_name    = _peer.name();
  _rhdr.msg_namelen = _peer.sizeofName();
}

VClientSocket::~VClientSocket()
{
}

int VClientSocket::set_dst(const Ins& dst,
			   int interface)
{
  in_addr address;
  address.s_addr = htonl(interface);
  int result = setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_IF,
 			  (char*)&address, sizeof(in_addr));
  if (result==-1)
    printf("MCS error setting multicast interface %x : reason %s\n",
 	   interface, strerror(errno));

  _addr.get(dst);

  sockaddr_in name;
  socklen_t name_len=sizeof(name);
  getsockname(_socket,(sockaddr*)&name,&name_len);

  return result;
}

