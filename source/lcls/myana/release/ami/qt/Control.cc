#include "ami/qt/Control.hh"

#include "ami/qt/Client.hh"
#include "ami/qt/Defaults.hh"
#include "ami/qt/QtPersistent.hh"
#include "ami/service/Task.hh"

#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QDoubleValidator>

using namespace Ami::Qt;

Control::Control(Requestor& c, double request_rate, bool hLayout) :
  QWidget(0),
  _client(c),
  _task  (new Task(TaskObject("amitmr")))
{
  if (hLayout) {
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(_pRun    = new QPushButton("Run")); _pRun->setCheckable(true);
    layout->addWidget(_pSingle = new QPushButton("Single"));
    layout->addStretch();
    layout->addWidget(new QLabel("Rate(Hz)"));
    layout->addWidget(_pRate   = new QLineEdit  (QString::number(request_rate)));
    _pRate->setMaximumWidth(40);
    setLayout(layout);
  }
  else {
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(_pRun    = new QPushButton("Run")); _pRun->setCheckable(true);
    layout->addWidget(_pSingle = new QPushButton("Single"));
    { QHBoxLayout* layout1 = new QHBoxLayout;
      layout1->addWidget(new QLabel("Rate(Hz)"));
      layout1->addWidget(_pRate   = new QLineEdit  (QString::number(request_rate)));
      _pRate->setMaximumWidth(40);
      layout->addLayout(layout1); }
    setLayout(layout);
  }

  _pRate->setValidator(new QDoubleValidator(0.1,5,1,_pRate));

  connect(_pRun   , SIGNAL(toggled(bool)), this, SLOT(run (bool)));
  connect(_pSingle, SIGNAL(clicked()), this, SLOT(single()));
  connect(_pRate  , SIGNAL(editingFinished()), this, SLOT(set_rate()));

  set_rate();
  _pRun->setChecked(Defaults::instance()->select_run());
}

Control::~Control()
{
  _task->destroy();
}

void Control::expired() { _client.request_payload(); }

Ami::Task* Control::task() { return _task; }

unsigned Control::duration() const { return _duration; }

unsigned Control::repetitive() const { return _repeat; }

void Control::run (bool l) {
  _client.one_shot(false);
  _repeat=1; 
  if (l)  Timer::start(); 
  else    cancel();
}

void Control::single() {
  _pRun->setChecked(false);
  _client.one_shot(true);
  _repeat = 0; 
  Timer::start();
}

void Control::set_rate() {
  _duration = static_cast<unsigned>(1000./_pRate->text().toDouble());
  if (_pRun->isChecked()) {
    cancel();
    Timer::start();
  }
}

void Control::save(char*& p) const
{
  XML_insert( p, "QPushButton", "_pRun",
              QtPersistent::insert(p,_pRun   ->isChecked()) );
}

void Control::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if (tag.name == "_pRun") {
      bool b = QtPersistent::extract_b(p);
      printf("Extract RUN state %c\n",b?'t':'f');
      _pRun   ->setChecked(b);
      run(b);
    }
  XML_iterate_close(Control,tag);
}
