#ifndef AmiQt_TdcClient_hh
#define AmiQt_TdcClient_hh

#include "ami/qt/AbsClient.hh"

#include "ami/data/Cds.hh"

#include <list>

class QButtonGroup;
class QComboBox;
class QPushButton;
class QLineEdit;

namespace Ami {
  class ClientManager;
  class DescEntry;
  class Semaphore;

  namespace Qt {
    class Control;
    class Status;
    class TdcPlot;
    class ProjectionPlot;
    class TwoDPlot;
    class DescTH1F;
    class DescTH2F;
    class Filter;
    class TdcClient : public Ami::Qt::AbsClient {
      Q_OBJECT
    public:
      enum { TH1F, TH2F, Image };
      TdcClient(QWidget*, const Pds::DetInfo&, unsigned);
      virtual ~TdcClient();
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
      void update_configuration();
      void _read_description   (int);
      void plot                ();
      void remove_plot         (QObject*);
      void select_source       ();
      void validate_source     ();
      void select_vsource      ();
      void validate_vsource    ();
    signals:
      void description_changed(int);
    private:
      void _select_source  (QLineEdit*);
      void _configure(char*& p, 
		      unsigned input, 
		      unsigned& output);
      void _setup_payload(Cds&);
      void _update();
    private:
      QString     _title;
      unsigned    _input_signature;
      unsigned    _output_signature;
      char*       _request;
      char*       _description;

      Control*    _control;
      Status*     _status;

      Cds             _cds;
      ClientManager*  _manager;
      unsigned        _niovload;
      iovec*          _iovload;

      Semaphore*  _sem;

      bool _throttled;

      QLineEdit*   _source_edit;
      QPushButton* _source_compose;

      QLineEdit*   _vsource_edit;
      QPushButton* _vsource_compose;

      Filter*      _filter;

      QButtonGroup* _plot_grp;
      DescTH1F*    _plot_desc_1d;
      DescTH2F*    _plot_desc_2d;

      std::list<TdcPlot*>        _plots;
      std::list<ProjectionPlot*> _pplots;
      std::list<TwoDPlot*>       _tplots;
    };
  };
};

#endif
