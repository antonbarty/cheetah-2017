#include "ami/service/BSocket.hh"

#include <sys/socket.h>
#include <string.h>
#include <stdio.h>

using namespace Ami;

BSocket::BSocket(unsigned size) :
  _size(size),
  _buffer(new char[size])
{
}

BSocket::~BSocket()
{
}

int BSocket::readv(const iovec* iov, int iovcnt)
{
  char* data = _buffer;
  char* end  = _buffer + _size;
  for(int i=0; i<iovcnt; i++) {
    if (data + iov[i].iov_len > end) {
      printf("BSocket::readv truncated at %d/%d item %d/%d\n",
	     data, end, i, iovcnt);
      break;
    }
    else {
      memcpy(iov[i].iov_base, data, iov[i].iov_len);
      data += iov[i].iov_len;
    }
  }
  int bytes = data-_buffer;
#ifdef DBUG
  { printf("\nBSocket %d read %d bytes\n",socket(),bytes);
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
  return bytes;
}

char* BSocket::data() { return _buffer; }
