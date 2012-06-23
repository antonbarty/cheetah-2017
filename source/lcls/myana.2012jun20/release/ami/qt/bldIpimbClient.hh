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
    class ServerEntry {
    public:
      char            name[32];
      char            host[32];
      unsigned        port;
      Pds::DetInfo    info;
    };

    typedef std::list<ServerEntry> SList;

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
      unsigned                   _sendEnb;	  
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

