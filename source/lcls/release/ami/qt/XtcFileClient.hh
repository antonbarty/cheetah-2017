#ifndef AmiQt_XtcFileClient_hh
#define AmiQt_XtcFileClient_hh

#include <QtGui/QWidget>
#include "ami/service/Routine.hh"

#include <list>

class QListWidget;
class QPushButton;

namespace Pds {
  class Dgram;
  class Xtc;
  class Sequence;
};

namespace Ami {

  class Task;
  class XtcClient;

  namespace Qt {
    class FileSelect;
    class XtcFileClient : public  QWidget,
			  public  Routine {
      Q_OBJECT
    public:
      XtcFileClient(Ami::XtcClient&  client,
		    const char* basedir);
      ~XtcFileClient();
    public:
      void routine();
      void configure();
    public slots:
      void select_expt(const QString&);
      void configure_run();
      void run();
      void ready();
    signals:
      void done();
    private:
      Ami::XtcClient&  _client;
      const char* _basedir;
      Task*       _task;  // thread for Qt
      FileSelect* _file_select;
      QListWidget* _expt_select;
      QPushButton* _runB;
    };
  };
}

#endif
