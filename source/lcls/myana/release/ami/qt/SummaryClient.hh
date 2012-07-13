#ifndef AmiQt_SummaryClient_hh
#define AmiQt_SummaryClient_hh

#include "ami/qt/AbsClient.hh"

#include "ami/data/Cds.hh"
#include "ami/data/ConfigureRequest.hh"

#include <list>

class QTabWidget;

namespace Ami {
  class ClientManager;
  class DescEntry;
  class Semaphore;
  class Control;
  class Status;

  namespace Qt {
    class Control;
    class Status;

    class SummaryClient : public AbsClient {
      Q_OBJECT
    public:
      SummaryClient(QWidget*,
		    const Pds::DetInfo&,
		    unsigned,
		    const QString&,
		    ConfigureRequest::Source);
      virtual ~SummaryClient();
    public:
      const QString& title() const;
      virtual void save(char*&) const;
      virtual void load(const char*&);
      void save_plots(const QString&) const;
      void reset_plots();
    public:
      void managed         (ClientManager&);
      void request_payload ();
      void one_shot        (bool) {}
    public: // AbsClient interface
      void connected       ();
      int  configure       (iovec*);
      int  configured      ();
      void discovered      (const DiscoveryRx&);
      void read_description(Ami::Socket&,int);
      void read_payload    (Ami::Socket&,int);
      void process         ();
    public slots:
      void _read_description   (int);
    signals:
      void description_changed(int);
    private:
      void _configure(char*& p, 
		      unsigned input, 
		      unsigned& output);
      void _setup_payload(Cds&);
      void _update();
    private:
      QString     _title;
      ConfigureRequest::Source _source;
      char*       _request;
      char*       _description;

      Cds             _cds;
      ClientManager*  _manager;
      unsigned        _niovload;
      iovec*          _iovload;

      Control*        _control;
      Status*         _status;
      QTabWidget*     _tab;

      Semaphore*  _sem;
    };
  };
};

#endif
