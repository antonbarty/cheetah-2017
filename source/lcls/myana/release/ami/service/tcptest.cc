#include "ami/service/TSocket.hh"
#include "ami/service/Sockaddr.hh"
#include "ami/service/Ins.hh"
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

using namespace Ami;

int main(int argc, char* argv[])
{
  bool listener=false;
  const char* host=0;
  unsigned short port=0;

  char c;
  while ((c = getopt(argc, argv, "lh:p:")) != -1) {
    switch(c) {
    case 'l': listener=true; break;
    case 'h': host=optarg; break;
    case 'p': port=strtoul(optarg, NULL, 0); break;
    }
  }

  char        rmsg[32];
  const char* smsg = "roger wilco...";
  int slen = strlen(smsg)+1;

  if (listener) {
    TSocket _listen;
    Ins ins(_listen.ins().address(),port);
    _listen.bind(ins);
    while(1) {
      if (::listen(_listen.socket(),5)<0)
	printf("Listen failed\n");
      else {

	printf("Listening on socket %d : %x/%d\n",
	       _listen.socket(),
	       _listen.ins().address(), _listen.ins().portId());

	Sockaddr name;
	unsigned length = name.sizeofName();
	int s = ::accept(_listen.socket(),name.name(), &length);
	if (s<0)
	  printf("Accept failed\n");
	else {
	  TSocket _accepted(s);
	  Ins local  = _accepted.ins();
	  Ins remote = name.get();
	  printf("new connection %d bound to local: %x/%d  remote: %x/%d\n",
	       s, 
	       local .address(), local .portId(),
	       remote.address(), remote.portId());

          _accepted.write(smsg, slen);
          printf("sent msg %s\n",smsg);

          int rlen = _accepted.read (rmsg, slen);
          printf("recv msg %s [%d]\n",rmsg,rlen);

	  sleep(2);
	  ::close(s);
	}
      }
    }
  }
  else {
    struct hostent* hent = gethostbyname(host);
    if (!hent) 
      printf("Failed to lookup entry for host %s\n",host);
    else {
      unsigned char* addr = (unsigned char*)hent->h_addr;
      unsigned ip_host = ntohl(*(unsigned*)hent->h_addr);
      printf("Lookup found host %s %d.%d.%d.%d %x\n",
	     host,
	     addr[0],addr[1],addr[2],addr[3],
	     ip_host);

      try {
	TSocket _talk;
	{ Ins local(port);
	  _talk.bind(local); }
	
	Ins remote(ip_host,port);
	_talk.connect(remote);
	Ins local (_talk.ins());
	
	printf("new connection %d bound to local: %x/%d  remote: %x/%d\n",
	       _talk.socket(), 
	       local .address(), local .portId(),
	       remote.address(), remote.portId());

        int rlen = _talk.read (rmsg, slen);
        printf("recv msg %s [%d]\n",rmsg,rlen);

        _talk.write(rmsg, rlen);
        printf("sent msg %s\n",rmsg);
      }
      catch (Event& e) {
	printf("Connected failed: %s\n", e.what());
      }
    }
  }
  return 0;
}
