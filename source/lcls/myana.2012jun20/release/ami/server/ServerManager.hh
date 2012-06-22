#ifndef Ami_ServerManager_hh
#define Ami_ServerManager_hh

#include "ami/service/Fd.hh"
#include "ami/service/Poll.hh"

#include <list>

namespace Ami {

  class Factory;
  class Ins;
  class Server;
  class Socket;
  class VServerSocket;

  class ServerManager : public Poll,
			public Fd {
  public:
    ServerManager (unsigned interface,
		   unsigned serverGroup);
    ~ServerManager();
  public:
    void serve     (Factory&);
    void dont_serve();
  public:
    void discover  ();
    void discover_post();
  public:
    void unmanage  (Fd&);
  private:   // Poll interface
    virtual int processTmo();
    virtual int processIn (const char*, int);
  public:    // Fd interface
    virtual int fd() const;
    virtual int processIo();
  private:
    typedef std::list<Server*> SvList;
    unsigned           _interface;
    unsigned           _serverGroup;
    unsigned           _ppinterface;
    Factory*           _factory;
    Socket*            _socket;
    SvList             _servers;
  };
};

#endif
