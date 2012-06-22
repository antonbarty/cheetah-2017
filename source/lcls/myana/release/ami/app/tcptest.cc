#include "ami/service/TSocket.hh"
#include "ami/service/Sockaddr.hh"

#include <stdio.h>

#include <list>
#include <poll.h>

using namespace Ami;

int main(int argc, char **argv) 
{
  unsigned interface   = 0x7f000001;
  unsigned port  = 0;
  bool lreceiver = false;

  for(int i=0; i<argc; i++) {
    if (strcmp(argv[i],"-i")==0) {
      interface = Ami::Ins::parse_interface(argv[++i]);
    }
    else if (strcmp(argv[i],"-p")==0) {
      port = strtoul(argv[++i],NULL,0);
    }
    else if (strcmp(argv[i],"-r")==0)
      lreceiver = true;
  }

  if (lreceiver) {
    TSocket* _listen = new TSocket;
    unsigned short _port = 0;
    while(_port != port) {
      _port = port;
      printf("Trying %x.%d\n",interface,port);
      try             { _listen->bind(Ins(interface,_port)); }
      catch(Event& e) { 
	printf("Failed to bind to port %d\n",port);
	++port;
      }
    }

    while(1) {

      ::listen(_listen->socket(),5);

      Ami::Sockaddr name;
      unsigned length = name.sizeofName();
      int s = ::accept(_listen->socket(),name.name(), &length);
      if (s<0)
	printf("accept failed\n");
      else {
	Ins remote = name.get();

	printf("accepting connection from %x.%d\n",
	       remote.address(),remote.portId());

	char buff[128];
	while(1) {
	  int len = ::read(s,buff,128);
	  if (len <= 0) {
	    printf("socket closed\n");
	    break;
	  }
	  
	  buff[len] = 0;
	  printf("=>%s\n",buff);
	}
	close(s);
      }
    }
    delete _listen;
  }

  else {
    TSocket* _send = new TSocket;
    _send->connect(Ins(interface,port));

    char buff[128];
    unsigned count=0;
    while(1) {
      sleep(1);
      int len = sprintf(buff,"message %d",count);
      _send->write(buff,len);
      count++;
    }
  }

}
