#ifndef Ami_Server
#define Ami_Server

#include "ami/service/VPoll.hh"
#include "ami/service/Fd.hh"

#include "ami/data/Cds.hh"
#include "ami/data/Message.hh"

class iovec;

namespace Ami {

  class VServerSocket;
  class VServerManager;
  class Factory;

  class Server : public VPoll,
		 public Fd {
  public:
    Server(VServerSocket*,
	   VServerManager&,
	   Factory&);
    ~Server();
  public:
    void discover();
  private:  // VPoll interface
    int processTmo();
  public:  // Fd interface
    int fd() const;
    int processIo();
  private:
    void _adjust     (int);
    void reply       (unsigned,Message::Type,unsigned);
  private:
    VServerSocket*  _socket;
    VServerManager& _mgr;
    iovec*          _iov;
    int             _iovcnt;
    Message         _reply;
    Cds             _cds;
    Factory&        _factory;
    char*           _buffer;
  };

};

#endif
