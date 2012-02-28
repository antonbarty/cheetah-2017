#include "TdcClient.hh"

#include "ami/qt/Control.hh"
#include "ami/qt/Status.hh"
#include "ami/qt/Calculator.hh"
#include "ami/qt/FeatureRegistry.hh"
#include "ami/qt/Filter.hh"
#include "ami/qt/TdcPlot.hh"
#include "ami/qt/DescTH1F.hh"

#include "ami/client/ClientManager.hh"

#include "ami/data/ChannelID.hh"
#include "ami/data/ConfigureRequest.hh"
#include "ami/data/Discovery.hh"
#include "ami/data/DescTH1F.hh"
#include "ami/data/EntryFactory.hh"
#include "ami/data/Expression.hh"
#include "ami/data/TdcPlot.hh"

#include "ami/service/Socket.hh"
#include "ami/service/Semaphore.hh"

#include <QtGui/QButtonGroup>
#include <QtGui/QRadioButton>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QGroupBox>
#include <QtGui/QComboBox>

#include <sys/types.h>
#include <sys/socket.h>

using namespace Ami::Qt;

static const int BufferSize = 0x8000;

TdcClient::TdcClient(QWidget* parent, const Pds::DetInfo& info, unsigned channel) :
  Ami::Qt::AbsClient(parent,info,channel),
  _title           (ChannelID::name(info,channel)),
  _input_signature (0),
  _output_signature(0),
  _request         (new char[BufferSize]),
  _description     (new char[BufferSize]),
  _cds             ("Client"),
  _manager         (0),
  _niovload        (5),
  _iovload         (new iovec[_niovload]),
  _sem             (new Semaphore(Semaphore::EMPTY)),
  _throttled       (false)
{
  setWindowTitle(ChannelID::name(info,channel));
  setAttribute(::Qt::WA_DeleteOnClose, false);

  _control = new Control(*this,1.);
  _status  = new Status;

  //  QPushButton* filterB = new QPushButton("Filter");
  _filter = new Filter     (NULL,_title);

  _source_edit    = new QLineEdit("");
  _source_compose = new QPushButton("Select");

  _plot_desc = new DescTH1F("Hist1D");
  _plot_desc->button()->setChecked(true);
  _plot_desc->button()->setEnabled(false);

  QPushButton* plotB  = new QPushButton("Plot");
  QPushButton* closeB = new QPushButton("Close");

  QVBoxLayout* layout = new QVBoxLayout;
  { QHBoxLayout* layout1 = new QHBoxLayout;
    layout1->addWidget(_control);
    layout1->addStretch();
    layout1->addWidget(_status);
    layout->addLayout(layout1); }
  { QGroupBox* channel_box = new QGroupBox("Source Channel");
    QHBoxLayout* layout1 = new QHBoxLayout;
    layout1->addWidget(_source_edit);
    layout1->addWidget(_source_compose);
    //    layout1->addWidget(filterB);
    channel_box->setLayout(layout1);
    layout->addWidget(channel_box); }
  { layout->addWidget(_plot_desc); }
  { QHBoxLayout* layout1 = new QHBoxLayout;
    layout1->addStretch();
    layout1->addWidget(plotB);
    layout1->addWidget(closeB);
    layout1->addStretch();
    layout->addLayout(layout1); }
  setLayout(layout);

  //  connect(filterB   , SIGNAL(clicked()),   _filter, SLOT(show()));
  connect(_source_edit   , SIGNAL(editingFinished()), this, SLOT(validate_source()));
  connect(_source_compose, SIGNAL(clicked()),         this, SLOT(select_source()));
  connect(plotB     , SIGNAL(clicked()),      this, SLOT(plot()));
  connect(closeB    , SIGNAL(clicked()),      this, SLOT(hide()));
  connect(this, SIGNAL(description_changed(int)), this, SLOT(_read_description(int)));
  connect((AbsClient*)this, SIGNAL(changed()), this, SLOT(update_configuration()));
}

TdcClient::~TdcClient() 
{
  if (_manager) delete _manager;
  delete[] _iovload;
  delete[] _description;
  delete[] _request;
  delete _filter;
}

const QString& TdcClient::title() const { return _title; }

void TdcClient::save(char*& p) const
{
  XML_insert(p, "QtPWidget", "self", QtPWidget::save(p) );

  XML_insert(p, "QLineEdit", "_source_edit", QtPersistent::insert(p,_source_edit->text()) );
  //  _filter->save(p);

  XML_insert(p, "DescTH1F", "_plot_desc", _plot_desc->save(p) );

  for(std::list<TdcPlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++) {
    XML_insert(p, "TdcPlot", "_plots", (*it)->save(p) );
  }

  XML_insert(p, "Control", "_control", _control->save(p) );
}

void TdcClient::load(const char*& p)
{
  for(std::list<TdcPlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++)
    disconnect(*it, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
  _plots.clear();

  XML_iterate_open(p,tag)
    if      (tag.element == "QtPWidget")
      QtPWidget::load(p);
    else if (tag.name == "_source_edit")
      _source_edit->setText(QtPersistent::extract_s(p));
  //  _filter  ->load(p);
    else if (tag.name == "_plot_desc")
      _plot_desc->load(p);
    else if (tag.name == "_plots") {
      TdcPlot* plot = new TdcPlot(this, p);
      _plots.push_back(plot);
      connect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
    }
    else if (tag.name == "_control")
      _control->load(p);
  XML_iterate_close(TdcClient,tag);

  update_configuration();
}

void TdcClient::save_plots(const QString& p) const 
{
  int i=1;
  for(std::list<TdcPlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++) {
    QString s = QString("%1_%2.dat").arg(p).arg(i++);
    FILE* f = fopen(qPrintable(s),"w");
    if (f) {
      (*it)->dump(f);
      fclose(f);
    }
  }
}

void TdcClient::reset_plots() { update_configuration(); }

void TdcClient::connected()
{
  if (_manager) {
    _status->set_state(Status::Connected);
    _manager->discover();
  }
}

void TdcClient::discovered(const DiscoveryRx& rx)
{
  _status->set_state(Status::Discovered);
  printf("Tdc Discovered\n");

  //  iterate through discovery and print
  FeatureRegistry::instance().insert(rx);

  char channel_name [128];
  strcpy(channel_name ,ChannelID::name(info,channel));
  const unsigned INIT = -1;
  _input_signature = INIT;

  for(  const DescEntry* e = rx.entries(); e < rx.end();
      e = reinterpret_cast<const DescEntry*>
        (reinterpret_cast<const char*>(e) + e->size())) {
    if (strcmp(e->name(),channel_name)==0) {
      _input_signature = e->signature();
      break;
    }
  }

  if (_input_signature==INIT)
    printf("%s not found\n", channel_name);

  _manager->configure();
}

int  TdcClient::configure       (iovec* iov) 
{
  _status->set_state(Status::Configured);
  printf("Configure\n");

  char* p = _request;

  for(std::list<TdcPlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++)
    (*it)->configure(p,_input_signature,_output_signature);

  if (p > _request+BufferSize) {
    printf("Client request overflow: size = 0x%x\n", p-_request);
    return 0;
  }
  else {
    iov[0].iov_base = _request;
    iov[0].iov_len  = p - _request;
    return 1;
  }
}

int  TdcClient::configured      () 
{
  printf("Tdc Configured\n");
  return 0; 
}

void TdcClient::read_description(Socket& socket,int len)
{
  printf("Tdc Described so\n");
  int size = socket.read(_description,len);

  if (size<0) {
    printf("Read error in Ami::Qt::Client::read_description.\n");
    return;
  }

  if (size==BufferSize) {
    printf("Buffer overflow in Ami::Qt::Client::read_description.  Dying...\n");
    abort();
  }

  //  printf("emit description\n");
  emit description_changed(size);

  _sem->take();
}

void TdcClient::_read_description(int size)
{
  printf("Tdc Described si\n");

  _cds.reset();

  const char* payload = _description;
  const char* const end = payload + size;
  //  dump(payload, size);

  //  { const Desc* e = reinterpret_cast<const Desc*>(payload);
  //    printf("Cds %s\n",e->name()); }
  payload += sizeof(Desc);

  while( payload < end ) {
    const DescEntry* desc = reinterpret_cast<const DescEntry*>(payload);
    if (desc->size()==0) {
      printf("read_description size==0\n");
      break;
    }
    Entry* entry = EntryFactory::entry(*desc);
    _cds.add(entry, desc->signature());
    payload += desc->size();
//     printf("Received desc %s signature %d\n",desc->name(),desc->signature());
  }

  if (_cds.totalentries()>_niovload) {
    delete[] _iovload;
    _iovload = new iovec[_niovload=_cds.totalentries()];
  }
  _cds.payload(_iovload);

  for(std::list<TdcPlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++)
    (*it)->setup_payload(_cds);

  _status->set_state(Status::Described);

  _sem->give();
}

void TdcClient::read_payload     (Socket& socket,int)
{
  //  printf("payload\n"); 
  if (_status->state() == Status::Requested) {
    socket.readv(_iovload,_cds.totalentries());
  }
  else {
    //
    //  Add together each server's contribution
    //
    printf("Ami::Qt::Client::read_payload: multiple server processing not implemented\n");
  }
  _status->set_state(Status::Received);
}

void TdcClient::process         () 
{
  //
  //  Perform client-side processing
  //
  for(std::list<TdcPlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++)
    (*it)->update();
  
  _status->set_state(Status::Processed);
}

void TdcClient::managed(ClientManager& mgr)
{
  _manager = &mgr;
  show();
  printf("TdcClient connecting\n");
  _manager->connect();
}

void TdcClient::request_payload()
{
  if (_plots.size()==0) return;

  if (_status->state() == Status::Described ||
      _status->state() == Status::Processed) {
    _throttled = false;
    _status->set_state(Status::Requested);
    _manager->request_payload();
  }
  else if (_status->state() == Status::Requested &&
           !_throttled) {
    _throttled = true;
    printf("TdcClient request_payload throttling\n");
  }
}

void TdcClient::update_configuration()
{
  if (_manager)
    _manager->configure();
}

void TdcClient::plot()
{
  //  Replace channel names with indices
  QString expr(_source_edit->text());
  const char* title = qPrintable(expr);

  Ami::DescTH1F* desc = new Ami::DescTH1F(title,
					  title, "hits",
					  _plot_desc->bins(),
					  _plot_desc->lo(),
					  _plot_desc->hi(),
					  false);
  
  for(unsigned i=0; i<6; i++)
    expr.replace(QString("Chan%1").arg(i+1),
		 QString("{%1}"  ).arg(i));
  
  printf("TdcPlot create expr %s\n",qPrintable(expr));

  TdcPlot* plot = new TdcPlot(this,
			      _source_edit->text(),
			      *_filter->filter(),
			      desc,
			      expr);

  _plots.push_back(plot);

  connect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));

  emit changed();
}

void TdcClient::remove_plot(QObject* obj)
{
  TdcPlot* plot = static_cast<TdcPlot*>(obj);
  _plots.remove(plot);

  disconnect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
}

void TdcClient::select_source()
{
  static QChar _exponentiate(0x005E);
  static QChar _multiply    (0x00D7);
  static QChar _divide      (0x00F7);
  static QChar _add         (0x002B);
  static QChar _subtract    (0x002D);

  QStringList vars;
  for(unsigned i=0; i<6; i++)
    vars << QString("Chan%1").arg(i+1);

  QStringList varops; varops << _subtract;
  QStringList conops; conops << _multiply << _divide << _add << _subtract;

  Calculator* c = new Calculator("Tdc Source","",
				 vars,
				 varops,
				 conops);
  if (c->exec()==QDialog::Accepted) {
    QString expr(c->result());
    expr.replace(_exponentiate,Expression::exponentiate());
    expr.replace(_multiply    ,Expression::multiply());
    expr.replace(_divide      ,Expression::divide());
    expr.replace(_add         ,Expression::add());
    expr.replace(_subtract    ,Expression::subtract());
    _source_edit->setText(expr);
  }
  delete c;
}

void TdcClient::validate_source()
{
  //  Don't know how to validate yet
}
