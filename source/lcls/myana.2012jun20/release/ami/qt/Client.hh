#ifndef AmiQt_Client_hh
#define AmiQt_Client_hh

//=========================================================
//
//  Client for the Analysis and Monitoring Implementation
//
//  Filter configures the server-side filtering
//  Math   configures the server-side operations
//  Control and Scale configure the client-side processing
//
//=========================================================

#include "ami/qt/AbsClient.hh"

#include "ami/data/Cds.hh"

class QLayout;

namespace Ami {
  class ClientManager;
  class DescEntry;
  class Semaphore;

  namespace Qt {
    class ChannelDefinition;
    class Control;
    class Display;
    class Status;

    class Client : public Ami::Qt::AbsClient {
      Q_OBJECT
    public:
      Client(QWidget*,const Pds::DetInfo&, unsigned, Display*, 
	     double request_rate=2.5);
      ~Client();
    public:
      const QString& title() const;
      void save(char*& p) const;
      void load(const char*& p);
      void reset_plots();
    public:
      void managed         (ClientManager&);
      void request_payload ();
      void one_shot        (bool);
    public:
      void connected       ();
      int  configure       (iovec*);
    public: // AbsClient interface
      int  configured      ();
      void discovered      (const DiscoveryRx&);
      void read_description(Ami::Socket&,int);
      void read_payload    (Ami::Socket&,int);
      void process         ();
    public slots:
      void update_configuration();
      void _read_description(int);
    signals:
      void description_changed(int);
    protected:
      void              addWidget(QWidget*);
      Ami::Qt::Display& display  ();
      const Ami::Qt::Display& display  () const;
    protected:
      virtual void _configure(char*& p, 
			      unsigned input, 
			      unsigned& output,
			      ChannelDefinition* ch[], 
			      int* signatures, 
			      unsigned nchannels) {}
      virtual void _setup_payload(Cds&) {}
      virtual void _update() {}
      virtual void _prototype(const DescEntry&) {}

    protected:
      enum {NCHANNELS=4};
      ChannelDefinition* _channels[NCHANNELS];
      Display*           _frame;
      const DescEntry*   _input_entry;

    private:
      QString     _title;
      unsigned    _output_signature;
      char*       _request;
      char*       _description;

      Control*    _control;
      Status*     _status;

      bool        _one_shot;

    protected:
      Cds             _cds;
    private:
      ClientManager*  _manager;
      unsigned        _niovload;
      iovec*          _iovload;

      QLayout*    _layout;

      Semaphore*  _sem;

      bool _throttled;
      unsigned _denials;
      unsigned _attempts;
    };
  };
};

#endif      
