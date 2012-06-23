#ifndef AmiQt_Control_hh
#define AmiQt_Control_hh

#include <QtGui/QWidget>
#include "ami/service/Timer.hh"

class QLabel;
class QLineEdit;
class QPushButton;

namespace Ami {
  namespace Qt {
    class Requestor;
    class Control : public QWidget,
		    public Timer {
      Q_OBJECT
    public:
      Control(Requestor&,double request_rate,bool lHLayout=true);
      ~Control();
    public:
      void save(char*&) const;
      void load(const char*&);
    public:  // Timer interface
      void     expired();
      Task*    task   ();
      unsigned duration  () const;
      unsigned repetitive() const;
    public slots:
      void run   (bool);
      void single();
      void set_rate();
    private:
      Requestor&  _client;
      Task*    _task;
      unsigned _repeat;
      unsigned _duration;
      QPushButton* _pRun;
      QPushButton* _pSingle;
      QLabel*      _status;
      QLineEdit*   _pRate;
    };
  };
};

#endif
