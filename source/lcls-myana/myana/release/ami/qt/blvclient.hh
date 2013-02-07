#include "ami/qt/ImageClient.hh"
#include "ami/qt/QtTopWidget.hh"
#include "ami/client/AbsClient.hh"
#include "ami/client/ClientManager.hh"
#include "ami/service/Ins.hh"
#include "ami/service/Port.hh"
#include "pdsapp/config/Experiment.hh"
#include "pdsdata/xtc/DetInfo.hh"

#include <QtGui/QListWidgetItem>
#include <QtGui/QListWidget>
#include <QtGui/QGroupBox>
#include <QtCore/QTimer>

class QComboBox;

namespace Pds_ConfigDb { 
  class Experiment;
  class Reconfig_Ui; 
}

namespace Ami {
  namespace Blv {
    class DetectorListItem : public QObject, 
			     public QListWidgetItem {
      Q_OBJECT
    public:
      DetectorListItem(QListWidget*        parent,
		       const QString&      dlabel,
		       const Pds::DetInfo& dinfo, 
		       unsigned            interface,
		       const char*         host,
		       unsigned            port );
      ~DetectorListItem() {}
    public:
      Pds::DetInfo    info;
    public slots:
      void enable (bool v) {
	if (v)
	  setFlags(flags() |  ::Qt::ItemIsEnabled); 
	else
	  setFlags(flags() & ~::Qt::ItemIsEnabled); 
      }
      void hide();
      void show();
    public:
      void reconnect() {
	if (_manager) {
	  if (_manager->nconnected()==0) {
	    enable(false);
	    _manager->connect();
	  }
	  else
	    enable(true);
	}
      }
    private:
      QWidget*           _parent;
      unsigned           _interface;
      unsigned           _port;
      Qt::ImageClient*   _client;
      unsigned           _ip_host;
      ClientManager*     _manager;
    };

    class ServerEntry {
    public:
      char            name[32];
      char            host[32];
      unsigned        port;
      Pds::DetInfo    info;
    };

    typedef std::list<ServerEntry> SList;

    class DetectorSelect : public QGroupBox {
      Q_OBJECT
    public:
      DetectorSelect(unsigned interface,
		     const SList& servers);
      ~DetectorSelect();
    public slots:
      void show_detector(QListWidgetItem*item) {
	if (_last_item)  _last_item->hide();
	_last_item = static_cast<DetectorListItem*>(item);
	_last_item->show();
      }
      void reconnect    () {
	for(int i=0; i<_detList->count(); i++)
	  static_cast<DetectorListItem*>(_detList->item(i))->reconnect();
      }
    private:
      unsigned short               _clientPort;
      QListWidget*                 _detList;
      DetectorListItem*            _last_item;
      QTimer*                      _reconnect_timer;
    };

    class ConfigSelect : public QGroupBox {
      Q_OBJECT
    public:
      ConfigSelect(unsigned          interface,
		   const SList&      servers,
		   const char*       db_path);
      ~ConfigSelect();
    public:
      void read_db();
    public slots:
      void set_run_type(const QString&); // a run type has been selected
      void update      ();  // the latest key for the selected run type has changed
    private:
      void _reconfigure();
    private:
      SList                      _servers;
      Pds_ConfigDb::Experiment   _expt;
      Pds_ConfigDb::Reconfig_Ui* _reconfig;
      QComboBox*                 _runType;
      unsigned                   _run_key;
    };

    class Control : public Qt::QtPWidget {
    public:
      Control(const QString&,
	      unsigned interface,
	      const SList& servers,
	      const char*  db);
    };
  };
};

