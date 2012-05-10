#ifndef Ami_VServerManager_hh
#define Ami_VServerManager_hh

#include "ami/service/Fd.hh"
#include "ami/service/Poll.hh"

#include <list>

namespace Ami {

  class Factory;
  class Ins;
  class VServer;
  class VServerSocket;

  class VServerManager : public Poll,
			 public Fd {
  public:
    VServerManager (unsigned interface,
		    Factory& factory);
    ~VServerManager();
  public:
    void serve     ();
    void dont_serve();
  public:
    void discover  ();
  public:
    void remove    (VServer*);
  private:   // VPoll interface
    virtual int processTmo();
  public:    // Fd interface
    virtual int fd() const;
    virtual int processIo();
  private:
    typedef std::list<VServer*> SvList;
    unsigned           _interface;
    Factory&           _factory;
    VServerSocket*     _socket;
    SvList             _servers;
  };

};

#endif
