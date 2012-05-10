#include "TSocket.hh"

#include "ami/data/Message.hh"
#include "ami/service/Sockaddr.hh"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

using namespace Ami;

TSocket::TSocket() throw(Event)
{
  if ((_socket = ::socket(AF_INET, SOCK_STREAM, 0)) < 0)
    throw Event("TSocket failed to open socket",strerror(errno));

  _rhdr.msg_name       = 0;
  _rhdr.msg_namelen    = 0;
  _rhdr.msg_control    = 0;
  _rhdr.msg_controllen = 0;
  _rhdr.msg_flags      = 0;
}

TSocket::TSocket(int s) throw(Event)
{
  _rhdr.msg_name       = 0;
  _rhdr.msg_namelen    = 0;
  _rhdr.msg_control    = 0;
  _rhdr.msg_controllen = 0;
  _rhdr.msg_flags      = 0;

  _socket = s;
}

//
//  Bind to port to listen
//
void TSocket::bind(const Ins& insb) throw(Event)
{
  _rhdr.msg_control    = 0;
  _rhdr.msg_controllen = 0;

  int optval=1;
  if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    throw Event("TSocket failed to set reuseaddr",strerror(errno));

  Sockaddr sa(insb);
  if (::bind(_socket, sa.name(), sa.sizeofName()) < 0)
    throw Event("TSocket failed to bind to port",strerror(errno));
}

Ins TSocket::ins() const throw(Event)
{
  Sockaddr name;
  socklen_t len = name.sizeofName();
  if (::getsockname(_socket, name.name(), &len) < 0)
    throw Event("TSocket failed to lookup name",strerror(errno));
  return name.get();
}

//
//  Connects to peer
//
void TSocket::connect(const Ins& peer) throw(Event) 
{
  Sockaddr sa(peer);
  if (::connect(_socket, sa.name(), sa.sizeofName())<0)
    throw Event("TSocket failed to connect to peer",strerror(errno));
}

TSocket::~TSocket()
{
}

int TSocket::readv(const iovec* iov, int iovcnt)
{
  _rhdr.msg_iov    = const_cast<iovec*>(iov);
  _rhdr.msg_iovlen = iovcnt;

  //  A read of 0 bytes + MSG_WAITALL = hang
  if (iov[0].iov_len==0) return 0;

  int bytes = ::recvmsg(_socket, &_rhdr, MSG_WAITALL);
#ifdef DBUG
  { printf("\nTSocket %d read %d bytes\n",socket(),bytes);
    int remaining=bytes;
    if (remaining>128) remaining=128;
    for(const iovec* i = iov; remaining>0; i++) {
      unsigned k=0;
      const unsigned char* end = (const unsigned char*)i->iov_base
	+ (i->iov_len > remaining ? remaining : i->iov_len);
      for(const unsigned char* c = (const unsigned char*)i->iov_base;
	  c < end; c++,k++)
	printf("%02x%c",*c,(k%32)==31 ? '\n' : ' ');
      printf("\n");
      remaining -= i->iov_len;
    }
  }
#endif
  if (bytes<0) {
    //    printf("Error reading from skt %d : %s\n",
    //	   socket(), strerror(errno));
    //    abort();
  }
  return bytes;
}

