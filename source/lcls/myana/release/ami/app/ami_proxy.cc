#include "ami/data/Message.hh"
#include "ami/client/VClientSocket.hh"
#include "ami/server/VServerSocket.hh"
#include "ami/service/Ins.hh"
#include "ami/service/Routine.hh"
#include "ami/service/Task.hh"
#include "ami/service/TaskObject.hh"
#include "ami/service/TSocket.hh"
#include "ami/service/Port.hh"
#include "ami/service/Sockaddr.hh"
#include "ami/service/Poll.hh"
#include "ami/service/Fd.hh"

#include <list>
#include <poll.h>

namespace Ami {

  class PSocket : public TSocket, public Fd {
  public:
    PSocket(int s, VClientSocket& v) : TSocket(s), _v(v) {}
    ~PSocket() {}
  public:
    int fd() const { return socket(); }
    int processIo() {
      //
      //  Forward a message received over tcp as a multicast
      //
      Message msg(0,Message::NoOp);

      int len = read(&msg,sizeof(Message));
      if (len)
	_v.write(&msg,len);

      return len;
    }
  private:
    VClientSocket& _v;
  };

  //
  //  A class to listen for servers that come online
  //
  class ConnectRoutine : public Routine {
  public:
    ConnectRoutine(Poll&   mgr,
		   Socket& skt) :
      _task(new Task(TaskObject("cmco"))),
      _mgr(mgr), _skt(skt), _found(false) 
    {
      _task->call(this); 
    }
    ~ConnectRoutine() {
      _task->destroy_b();
    }
  public:
    void routine()
    {
      pollfd fds[1];
      fds[0].fd = _skt.socket();
      fds[0].events = POLLIN | POLLERR;

      if (poll(fds, 1, 1000)>0) {
        Message msg(0,Message::NoOp);
        if (_skt.read(&msg,sizeof(msg))==sizeof(msg))
          if (msg.type()==Message::Hello) {
            printf("Hello from socket %d\n",
		   _skt.socket());
	    _mgr.bcast_out(reinterpret_cast<char*>(&msg),sizeof(Message));
            _found = true;
          }
      }
      else if (_found) {
        _found = false;
      }
      _task->call(this);
    }
  private:
    Task*           _task;
    Poll&           _mgr;
    Socket&         _skt;
    bool            _found;
  };
};


using namespace Ami;

int main(int argc, char **argv) 
{
  unsigned ppinterface = 0x7f000001;
  unsigned interface   = 0x7f000001;
  unsigned serverGroup = 0xefff2000;

  for(int i=0; i<argc; i++) {
    if (strcmp(argv[i],"-I")==0) {
      ppinterface = Ami::Ins::parse_interface(argv[++i]);
    }
    else if (strcmp(argv[i],"-i")==0) {
      interface = Ami::Ins::parse_interface(argv[++i]);
    }
    else if (strcmp(argv[i],"-s")==0) {
      serverGroup = Ami::Ins::parse_ip(argv[++i]);
    }
  }

  Ins mcast(serverGroup, Port::serverPort());
  VServerSocket server(mcast, interface);
  VClientSocket client; client.set_dst(mcast, interface);

  Poll clients(1000);
  clients.start();

  ConnectRoutine conn(clients, server);

  TSocket* _listen = new TSocket;
  unsigned short port  = Port::serverPort();
  try  { _listen->bind(Ins(ppinterface,port)); }
  catch(Event& e) { 
    printf("Failed to bind to port %d\n",port);
    exit(1);
  }

  while(1) {

    ::listen(_listen->socket(),5);

    Ami::Sockaddr name;
    unsigned length = name.sizeofName();
    int s = ::accept(_listen->socket(),name.name(), &length);
    if (s<0)
      printf("ami_proxy accept failed\n");
    else {
      Ins remote = name.get();

      printf("ami_proxy accepting connection from %x.%d\n",
	     remote.address(),remote.portId());

      clients.stop();
      clients.manage(*new PSocket(s,client));
      clients.start();
    }
  }

  delete _listen;
}
