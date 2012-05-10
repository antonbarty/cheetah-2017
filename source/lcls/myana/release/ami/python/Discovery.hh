#ifndef AmiPy_Discovery_hh
#define AmiPy_Discovery_hh

#include "ami/client/AbsClient.hh"
#include "pdsdata/xtc/DetInfo.hh"

#include <list>

namespace Ami {
  class ClientManager;
  namespace Python {
    class Discovery : public Ami::AbsClient {
    public:
      Discovery(unsigned ppinterface,
		unsigned interface,
		unsigned serverGroup);
      ~Discovery();
    public:
      Ami::ClientManager* allocate(Ami::AbsClient&);
    public:
      void connected       () ;
      int  configure       (iovec*) ;
      int  configured      () ;
      void discovered      (const DiscoveryRx&) ;
      void read_description(Socket&,int) ;
      void read_payload    (Socket&,int) ;
      void process         () ;

    private:
      unsigned       _ppinterface;
      unsigned       _interface;
      unsigned       _serverGroup;
      unsigned short _clientPort;
      ClientManager* _manager;
    };
  };
};

#endif
