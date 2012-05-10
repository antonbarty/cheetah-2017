#ifndef AmiPython_Client_hh
#define AmiPython_Client_hh

#include "ami/client/AbsClient.hh"

#include "ami/data/Cds.hh"

#include "pdsdata/xtc/DetInfo.hh"

#include <list>

namespace Ami {
  class AbsFilter;
  class AbsOperator;
  class ClientManager;
  class DescEntry;
  class Entry;

  namespace Python {
    class Client : public Ami::AbsClient {
    public:
      Client(const Pds::DetInfo& info, 
	     unsigned            channel,
	     AbsFilter*          filter,
	     AbsOperator*        op);
      virtual ~Client();
    public:
      enum { Success, TimedOut, NoEntry };
      int  initialize      (ClientManager&);
      int  request_payload ();
      const Entry* payload () const;
      void reset           ();
    public: // AbsClient interface
      void managed         (ClientManager&);
      void connected       ();
      int  configure       (iovec*);
      int  configured      ();
      void discovered      (const DiscoveryRx&);
      void read_description(Ami::Socket&,int);
      void read_payload    (Ami::Socket&,int);
      void process         ();
    private:
      Pds::DetInfo        _info;
      unsigned            _channel;
      AbsFilter*          _filter;
      AbsOperator*        _op;

      unsigned    _input;
      unsigned    _output_signature;
      char*       _request;
      char*       _description;

      Cds             _cds;
      ClientManager*  _manager;
      unsigned        _niovload;
      iovec*          _iovload;

      sem_t           _initial_sem;
      sem_t           _payload_sem;
    };
  };
};

#endif
