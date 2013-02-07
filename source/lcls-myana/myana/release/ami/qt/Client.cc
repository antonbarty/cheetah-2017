#include "Client.hh"

#include "ami/qt/Control.hh"
#include "ami/qt/Filter.hh"
#include "ami/qt/Display.hh"
#include "ami/qt/Status.hh"
#include "ami/qt/QtBase.hh"
#include "ami/qt/ChannelDefinition.hh"
#include "ami/qt/FeatureRegistry.hh"

#include "ami/client/ClientManager.hh"
#include "ami/data/ChannelID.hh"
#include "ami/data/ConfigureRequest.hh"
#include "ami/data/Discovery.hh"
#include "ami/data/DescEntry.hh"
#include "ami/data/EntryFactory.hh"

#include "ami/service/Socket.hh"
#include "ami/service/Semaphore.hh"

#include "pdsdata/xtc/ClockTime.hh"

#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QGroupBox>

#include <sys/types.h>
#include <sys/socket.h>

using namespace Ami;

typedef Pds::DetInfo DI;

static const int BufferSize = 0x40000; // 256 kB

static const bool oldLayout=false;

Ami::Qt::Client::Client(QWidget*            parent,
			const Pds::DetInfo& src,
			unsigned            channel,
			Display*            frame,
			double              request_rate) :
  Ami::Qt::AbsClient(parent,src,channel),
  _frame           (frame),
  _input_entry     (0),
  _title           (ChannelID::name(src,channel)),
  _output_signature(0), 
  _request         (new char[BufferSize]),
  _description     (new char[BufferSize]),
  _cds             ("Client"),
  _manager         (0),
  _niovload        (5),
  _iovload         (new iovec[_niovload]),
  _layout          (new QVBoxLayout),
  _sem             (new Semaphore(Semaphore::EMPTY)),
  _throttled       (false),
  _denials         (0),
  _attempts        (0)
{
  setWindowTitle(ChannelID::name(src, channel));

  setAttribute(::Qt::WA_DeleteOnClose, false);

  _control = new Control(*this,request_rate,oldLayout);
  _status  = new Status;

  QButtonGroup* showPlotBoxes = new QButtonGroup;
  showPlotBoxes->setExclusive( !frame->canOverlay() );

  QStringList names;
  for(unsigned i=0; i<NCHANNELS; i++)
    names << QString("%1_Ch%2").arg(_title).arg(char('A'+i));

  QStringList refnames;
  unsigned ref_mask = channel>>16;
  for(unsigned i=0; ref_mask!=0; i++) {
    if (ref_mask&1)
      refnames << QString("Chan %1").arg(i+1);
    ref_mask>>=1;
  }
  
  QHBoxLayout* layout = new QHBoxLayout;
  { QVBoxLayout* layout3 = new QVBoxLayout;
    if (!oldLayout) {
      { QGroupBox* ctrlBox = new QGroupBox("Control");
        QVBoxLayout* layout1 = new QVBoxLayout;
        layout1->addWidget(_control);
        layout1->addWidget(_status); 
        ctrlBox->setLayout(layout1);
        layout3->addWidget(ctrlBox); }
    }
    { QGroupBox* chanBox = new QGroupBox("Channels");
      QVBoxLayout* layout1 = new QVBoxLayout;
      QPushButton* chanB[NCHANNELS];
      QColor color[] = { QColor(0,0,255), QColor(255,0,0), QColor(0,255,0), QColor(255,0,255) };
      for(int i=0; i<NCHANNELS; i++) {
	QString title = names[i];
	_channels[i] = new ChannelDefinition(this,title, names, *_frame, color[i], i==0, refnames);
	chanB[i] = new QPushButton(QString("Ch%1").arg(char('A'+i))); chanB[i]->setCheckable(false);
	chanB[i]->setPalette(QPalette(color[i]));
	{ QHBoxLayout* layout4 = new QHBoxLayout;
	  QCheckBox* box = new QCheckBox("");
	  showPlotBoxes->addButton(box);
	  connect(box, SIGNAL(toggled(bool)), _channels[i], SLOT(show_plot(bool)));
	  connect(_channels[i], SIGNAL(show_plot_changed(bool)), box, SLOT(setChecked(bool)));
	  box->setChecked( i==0 );
	  layout4->addWidget(box);
	  layout4->addWidget(chanB[i]);
	  layout1->addLayout(layout4); 
	  connect(chanB[i], SIGNAL(clicked()), _channels[i], SLOT(show()));
	  connect(_channels[i], SIGNAL(changed()), this, SIGNAL(changed()));
	  connect(_channels[i], SIGNAL(newplot(bool)), box , SLOT(setChecked(bool))); 
	}
      }
      chanBox->setLayout(layout1);
      layout3->addWidget(chanBox); }
    layout3->addLayout(_layout);
    layout3->addStretch();
    layout->addLayout(layout3,0); }
  if (oldLayout) {
    { QVBoxLayout* layout1 = new QVBoxLayout;
      { QHBoxLayout* layout2 = new QHBoxLayout;
        layout2->addWidget(_control);
        layout2->addStretch();
        layout2->addWidget(_status);
        layout1->addLayout(layout2); }
      layout1->addWidget(_frame->widget());
      layout->addLayout(layout1,1); }
  }
  else {
    layout->addWidget(_frame->widget(),1);
  }
  setLayout(layout);

  connect(this, SIGNAL(description_changed(int)), this, SLOT(_read_description(int)));
  connect((AbsClient*)this, SIGNAL(changed()),    this, SLOT(update_configuration()));
}

Ami::Qt::Client::~Client() 
{
  if (_manager) delete _manager;
  delete[] _iovload;
  delete[] _description;
  delete[] _request; 
  delete _sem;
}

const QString& Ami::Qt::Client::title() const { return _title; }

void Ami::Qt::Client::save(char*& p) const
{
  XML_insert(p, "QtPWidget", "self", QtPWidget::save(p) );

  for(unsigned i=0; i<NCHANNELS; i++) 
    XML_insert(p, "ChannelDef", "_channels", _channels[i]->save(p) );

  XML_insert(p, "Display", "_frame", _frame->save(p) );
  XML_insert(p, "Control", "_control", _control->save(p) );
}

void Ami::Qt::Client::load(const char*& p)
{
  for(unsigned i=0; i<NCHANNELS; i++) {
    disconnect(_channels[i], SIGNAL(changed()), (AbsClient*)this, SIGNAL(changed()));
  }

  unsigned nchannels = 0;

  XML_iterate_open(p,tag)
    if (tag.element == "QtPWidget")
      QtPWidget::load(p);
    else if (tag.name == "_channels") {
      _channels[nchannels]->load(p);
      connect(_channels[nchannels], SIGNAL(changed()), (AbsClient*)this, SIGNAL(changed()));
      nchannels++;
    }
    else if (tag.name == "_frame")
      _frame->load(p);
    else if (tag.name == "_control")
      _control->load(p);
  XML_iterate_close(Client,tag);
}

void Ami::Qt::Client::reset_plots() { update_configuration(); }

void Ami::Qt::Client::addWidget(QWidget* w) { _layout->addWidget(w); }

Ami::Qt::Display& Ami::Qt::Client::display() { return *_frame; }

const Ami::Qt::Display& Ami::Qt::Client::display() const { return *_frame; }

void Ami::Qt::Client::connected()
{
  if (_manager) {
    _status->set_state(Status::Connected);
    _manager->discover();
  }
}

void Ami::Qt::Client::discovered(const DiscoveryRx& rx)
{
  _status->set_state(Status::Discovered);
  printf("%s Discovered\n",qPrintable(title()));

  //  iterate through discovery and print
  FeatureRegistry::instance().insert(rx.features());

  char channel_name [128]; 
  strcpy(channel_name ,ChannelID::name(info,channel));
  _input_entry = 0;

  for(  const DescEntry* e = rx.entries(); e < rx.end(); 
      e = reinterpret_cast<const DescEntry*>
	(reinterpret_cast<const char*>(e) + e->size())) {
    //    printf("Test %s\n",e->name());
    if (strcmp(e->name(),channel_name)==0) {
      _input_entry = e;
      _frame->prototype(e);
      _prototype(*e);
    }
  }

  if (_input_entry==0)
    printf("%s not found\n", channel_name);

  _manager->configure();
}

int  Ami::Qt::Client::configure       (iovec* iov) 
{
  _status->set_state(Status::Configured);
  printf("%s Configure\n",qPrintable(title()));
  if (_input_entry==0) {
    printf("input_entry not found\n");
    return 0;
  }
  else {

    int signatures[NCHANNELS];
    for(int i=0; i<NCHANNELS; i++)
      signatures[i] = -1;

    char* p = _request;

    //
    //  Configure channels which depend upon others
    //
    bool lAdded;
    do {
      lAdded=false;
      for(unsigned i=0; i<NCHANNELS; i++) {
	if (signatures[i]<0) {
	  int sig = _channels[i]->configure(p,_input_entry->signature(),_output_signature,
					    _channels,signatures,NCHANNELS);
	  if (sig >= 0) {
	    signatures[i] = sig;
	    lAdded = true;
	    //	    printf("Added signature %d for channel %d\n",sig,i);
	  }
	}
      }
    } while(lAdded);

    _configure(p,_input_entry->signature(),_output_signature,
	       _channels,signatures,NCHANNELS);

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
}

//
//  This state is unnecessary
//
int  Ami::Qt::Client::configured      () 
{
  printf("%s Configured\n",qPrintable(title()));
  return 0; 
}

//
//  read_description adds/removes plots
//
void Ami::Qt::Client::read_description(Socket& socket, int len)
{
  printf("%s Described so\n",qPrintable(_title));
  int size = socket.read(_description,len);

  if (size<0) {
    printf("Read error in Ami::Qt::Client::read_description.\n");
    return;
  }

  if (size>BufferSize) {
    printf("Buffer overflow [%d/%d] in Ami::Qt::Client::read_description.  Dying...\n",
	   size,BufferSize);
    abort();
  }

  //  printf("emit description\n");
  emit description_changed(size);

  _sem->take();
}

void Ami::Qt::Client::_read_description(int size)
{
  printf("%s Described si\n",qPrintable(_title));
  //  printf("description\n"); 

  _frame->reset();
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

  for(unsigned i=0; i<NCHANNELS; i++)
    _channels[i]->setup_payload(_cds);

  _setup_payload(_cds);

  _status->set_state(Status::Described);

  _sem->give();
}

//
//  read_payload changes plot contents
//
void Ami::Qt::Client::read_payload     (Socket& socket, int size)
{
  if (_status->state() == Status::Requested) {
    socket.readv(_iovload,_cds.totalentries());
  }
  else if (!_one_shot) {
    //
    //  Add together each server's contribution
    //
    printf("Ami::Qt::Client::read_payload: multiple server processing not implemented\n");
  }
  _status->set_state(Status::Received);
}

void Ami::Qt::Client::process         () 
{
  //
  //  Perform client-side processing
  //
  _frame  ->update();
  _update();
  
  _status->set_state(Status::Processed);
}

void Ami::Qt::Client::managed(ClientManager& mgr)
{
  _manager = &mgr;
  show();
  _manager->connect();
}

void Ami::Qt::Client::request_payload()
{
  _attempts++;
  if (_status->state() == Status::Described ||
      _status->state() == Status::Processed) {
    _throttled = false;
    _status->set_state(Status::Requested);
    _manager->request_payload();
  }
  else if (_status->state() == Status::Requested) {
    _denials++;
    if (!_throttled)
      _throttled = true;
    if ((_denials%20)==1)
      printf("Client %s request_payload throttled %d/%d\n",
	     qPrintable(_title),_denials,_attempts);
  }
}

void Ami::Qt::Client::one_shot(bool l) 
{
  _one_shot=l; 
}

void Ami::Qt::Client::update_configuration()
{
  if (_manager)
    _manager->configure();
}
