#include "DetectorSelect.hh"

#include "ami/qt/QtTopWidget.hh"
#include "ami/qt/WaveformClient.hh"
#include "ami/qt/ImageClient.hh"
#include "ami/qt/CspadClient.hh"
#include "ami/qt/EnvClient.hh"
#include "ami/qt/TdcClient.hh"
#include "ami/qt/SummaryClient.hh"
#include "ami/qt/Path.hh"
#include "ami/qt/PrintAction.hh"
#include "ami/qt/DetectorListItem.hh"
#include "ami/qt/Defaults.hh"
#include "ami/qt/FilterSetup.hh"
#include "ami/qt/RateDisplay.hh"
#include "ami/client/ClientManager.hh"
#include "ami/data/DescEntry.hh"
#include "ami/data/Discovery.hh"
#include "ami/data/ConfigureRequest.hh"
#include "ami/service/Port.hh"

#include "pdsdata/xtc/BldInfo.hh"

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QFont>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>
#include <QtCore/QTimer>

#include <errno.h>

using namespace Ami::Qt;

static const Pds::DetInfo envInfo(0,Pds::DetInfo::NoDetector,0,Pds::DetInfo::Evr,0);
static const Pds::DetInfo noInfo (0,Pds::DetInfo::NoDetector,0,Pds::DetInfo::NoDevice,0);

static const int MaxConfigSize = 0x100000;
static const int BufferSize = 0x8000;

//
//  Really need to replace DetInfo with Src in all monitoring usage
//  Until then, add level to the match criteria
//
static bool match(const Pds::Src& a,
                  const Pds::Src& b)
{
  return a.level()==b.level() && a.phy()==b.phy();
}

class SrcV : public Pds::Src {
public:
  SrcV(uint32_t l, uint32_t p) { _log=l; _phy=p; }
};


DetectorSelect::DetectorSelect(const QString& label,
                               unsigned ppinterface,
                               unsigned interface,
                               unsigned serverGroup,
                               QGroupBox* guestBox,
                               bool quiet) :
  QtPWidget   (0),
  //  _quiet      (quiet),
  _quiet      (false),
  _ppinterface(ppinterface),
  _interface  (interface),
  _serverGroup(serverGroup),
  _clientPort (Port::clientPortBase()),
  _manager    (new ClientManager(ppinterface,
         interface, serverGroup, 
         _clientPort++,*this)),
  _filters    (new FilterSetup(*_manager)),
  _request    (new char[BufferSize]),
  _rate_display(new RateDisplay(_manager)),
  _discovered(false)
{
  pthread_mutex_init(&_mutex, NULL);
  pthread_cond_init(&_condition, NULL);
  setWindowTitle(label);
  setAttribute(::Qt::WA_DeleteOnClose, false);

  QVBoxLayout* l = new QVBoxLayout;

  // Caller can add extra controls here.
  if (guestBox) {
    l->addWidget(guestBox);
  }

  { QGroupBox* setup_box = new QGroupBox("Setup");
    QVBoxLayout* layout = new QVBoxLayout;
    QPushButton* saveB    = new QPushButton("Save");
    QPushButton* loadB    = new QPushButton("Load");
    QPushButton* defaultB = new QPushButton("Defaults");
    //    QPushButton* printB   = new QPushButton("Printer");
    //    QPushButton* testB    = new QPushButton("Test");
    layout->addWidget(saveB);
    layout->addWidget(loadB);
    layout->addWidget(defaultB);
    //    layout->addWidget(testB);
    //    layout->addWidget(printB);
    connect(saveB   , SIGNAL(clicked()), this, SLOT(save_setup()));
    connect(loadB   , SIGNAL(clicked()), this, SLOT(load_setup()));
    //    connect(printB  , SIGNAL(clicked()), this, SLOT(print_setup()));
    connect(defaultB, SIGNAL(clicked()), this, SLOT(default_setup()));
    setup_box->setLayout(layout);
    l->addWidget(setup_box); }
  { QGroupBox* data_box  = new QGroupBox("Data");
    QVBoxLayout* layout = new QVBoxLayout;    

    QPushButton* resetB  = new QPushButton("Reset Plots");
    QPushButton* saveB   = new QPushButton("Save Plots");
    QPushButton* filterB = new QPushButton("Event Filter"); 
    layout->addWidget(resetB);
    layout->addWidget(saveB);
    layout->addWidget(filterB);
    connect(resetB , SIGNAL(clicked()), this, SLOT(reset_plots()));
    connect(saveB  , SIGNAL(clicked()), this, SLOT(save_plots()));
    connect(filterB, SIGNAL(clicked()), this, SLOT(set_filters()));

    layout->addWidget(_detList = new QListWidget(this));
    *new DetectorListItem(_detList, "Env"    , envInfo, 0);
    *new DetectorListItem(_detList, "PostAnalysis", envInfo, 1);
    //    *new DetectorListItem(_detList, "Summary", noInfo , 0);
    connect(_detList, SIGNAL(itemClicked(QListWidgetItem*)), 
      this, SLOT(show_detector(QListWidgetItem*)));

    data_box->setLayout(layout);
    l->addWidget(data_box); }
  _rate_display->addLayout(l);
  setLayout(l);

  connect(this, SIGNAL(detectors_discovered(const char*)), this, SLOT(change_detectors(const char*)));

  //  autoload();

  _autosave_timer = new QTimer(this);
  _autosave_timer->setSingleShot(true);
  connect(_autosave_timer, SIGNAL(timeout()), this, SLOT(autosave()));

  _manager->connect();
}

DetectorSelect::~DetectorSelect()
{
  for(std::list<QtTopWidget*>::iterator it = _client.begin();
      it != _client.end(); it++)
    if (*it) {
      disconnect((*it), SIGNAL(changed()), this, SLOT(queue_autosave()));
      delete (*it);
    }

  delete _rate_display;
  delete _filters;
  delete _manager;
  delete[] _request;
}

int DetectorSelect::get_setup(char* buffer) const
{
  char id[64];

  char* p = buffer;

  for(std::list<QtTopWidget*>::const_iterator it = _client.begin();
      it != _client.end(); it++)
    if ((*it)) {
      sprintf(id, "%08x.%08x.%d", (*it)->info.log(), (*it)->info.phy(), (*it)->channel);
      XML_insert( p, id, qPrintable((*it)->title()), (*it)->save(p) );
    }

  XML_insert( p, "QtTopWidget", "self", save(p) );

  return p-buffer;
}

void DetectorSelect::save_setup ()
{
  char* buffer = new char[MaxConfigSize];

  int len = get_setup(buffer);
  if (len > MaxConfigSize) {
    printf("DetectorSelect::save_setup exceeded save buffer size (0x%x/0x%x)\n",
     len, MaxConfigSize);
  }

  char time_buffer[32];
  time_t seq_tm = time(NULL);
  strftime(time_buffer,32,"%Y%m%d_%H%M%S",localtime(&seq_tm));

  QString def = QString("%1/%2.ami").arg(Path::base()).arg(time_buffer);
  QString fname =     
    QFileDialog::getSaveFileName(this,"Save Setup to File (.ami)",
                                 def,"*.ami");
  FILE* o = fopen(qPrintable(fname),"w");
  if (o) {
    fwrite(buffer,len,1,o);
    fclose(o);
    printf("Saved %d bytes to %s\n",len,qPrintable(fname));
  }
  else {
    QString msg = QString("Error opening %1 : %2").arg(fname).arg(strerror(errno));
    QMessageBox::critical(this,"Save Error",msg);
  }

  delete[] buffer;
}

void DetectorSelect::load_setup ()
{
  // get the file 
  QString fname = QFileDialog::getOpenFileName(this,"Load Setup from File (.ami)",
                 Path::base(), "*.ami");

  load_setup(qPrintable(fname));
}

void DetectorSelect::load_setup(const char* fname)
{
  FILE* f = fopen(fname,"r");
  if (!f) {
    QString msg = QString("Error opening %1 : %2").arg(fname).arg(strerror(errno));
    QMessageBox::critical(this,"Load Error",msg);
    return;
  }
  char* buffer = new char[MaxConfigSize];
  int size = fread(buffer,1,MaxConfigSize,f);
  fclose(f);

  printf("Load %d bytes from %s @ %p\n",size,fname,buffer);

  //
  //  Terminate the file
  //
  size += sprintf(buffer+size,"</Document>");

  set_setup(buffer, size);

  delete[] buffer;
}

void DetectorSelect::set_setup(const char* p, int size)
{
  const char* start = p;
  //
  // parse the input
  //
  //  Create the clients, if necessary, load them, and connect
  //
  for(Ami::XML::TagIterator it(p); !it.end(); it++) {
    const Ami::XML::StartTag& tag = *it;

    printf("DetectorSelect tag %s/%s [%d]\n",
           tag.element.c_str(),tag.name.c_str(),p-start);

    if (tag.name == "self")
      load(p);
    else {

      uint32_t log, phy, channel;
      if (sscanf(tag.element.c_str(),"%08x.%08x.%d",&log,&phy,&channel)!=3) {
        log = 0x01000000;
        sscanf(tag.element.c_str(),"%08x.%d",&phy,&channel);
      }

      printf("Seeking %s (%08x.%08x.%d)\n",tag.name.c_str(),log,phy,channel);

      SrcV info(log,phy);
      
      bool lFound=false;
      for(std::list<QtTopWidget*>::iterator it = _client.begin();
          it != _client.end(); it++)
        if (match((*it)->info,info) && (*it)->channel == channel) {
          printf("Loading %s\n",tag.name.c_str());
          lFound=true;
          (*it)->load(p);
          break;
        }
    
      if (!lFound) {
        _create_client(info,channel,tag.name.c_str(),p);
      }
    }
  }

}

void DetectorSelect::print_setup()
{
  QPrintDialog* d = new QPrintDialog(PrintAction::printer(),this);
  d->exec();
  delete d;
}

void DetectorSelect::default_setup()
{
  Defaults::instance()->show();
}

void DetectorSelect::set_filters()
{
  _filters->show();
}

void DetectorSelect::reset_plots()
{
  // _reset_box->apply();

  for(std::list<QtTopWidget*>::iterator it=_client.begin();
      it!=_client.end(); it++)
    (*it)->reset_plots();
}

void DetectorSelect::save_plots()
{
  // _save_box->apply();

  char time_buffer[32];
  time_t seq_tm = time(NULL);
  strftime(time_buffer,32,"%Y%m%d_%H%M%S",localtime(&seq_tm));

  QString def = QString("%1/%2").arg(Path::base()).arg(time_buffer);

  QString prefix = QFileDialog::getSaveFileName(0,"Save Files with Prefix",
            def,"*.dat");
  for(std::list<QtTopWidget*>::iterator it=_client.begin();
      it!=_client.end(); it++)
    (*it)->save_plots(QString("%1_%2").arg(prefix).arg((*it)->title()));
}

Ami::Qt::AbsClient* DetectorSelect::_create_client(const Pds::Src& src, 
               unsigned channel,
                                                   const QString& name,
                                                   const char*& p)
{
  const Pds::DetInfo& info = static_cast<const Pds::DetInfo&>(src);

  printf("Creating client for %s [%08x.%08x]\n",qPrintable(name), src.log(), src.phy());

  Ami::Qt::AbsClient* client = 0;
  if (info.level()==Pds::Level::Source) {
    switch(info.device()) {
    case Pds::DetInfo::NoDevice : client = new Ami::Qt::SummaryClient (this, info , channel, "Summary", ConfigureRequest::Summary); break;
    case Pds::DetInfo::Evr      : client = new Ami::Qt::EnvClient     (this, info, channel); break;
    case Pds::DetInfo::OceanOptics : 
    case Pds::DetInfo::Acqiris  : client = new Ami::Qt::WaveformClient(this, info, channel); break;
    case Pds::DetInfo::AcqTDC   : client = new Ami::Qt::TdcClient     (this, info, channel); break;
    case Pds::DetInfo::Opal1000 : 
    case Pds::DetInfo::Opal2000 : 
    case Pds::DetInfo::Opal4000 : 
    case Pds::DetInfo::Phasics  :
    case Pds::DetInfo::Timepix  :
    case Pds::DetInfo::TM6740   : 
    case Pds::DetInfo::pnCCD    :
    case Pds::DetInfo::Princeton: 
    case Pds::DetInfo::Fli      : 
    case Pds::DetInfo::Fccd     : client = new Ami::Qt::ImageClient   (this, info, channel); break;
    case Pds::DetInfo::Cspad    :
    case Pds::DetInfo::Cspad2x2 : client = new Ami::Qt::CspadClient   (this, info, channel); break;
    default: printf("Device type %x not recognized\n", info.device()); break;
    }
  }
  else if (src.level()==Pds::Level::Reporter) {
    const Pds::BldInfo& bld = reinterpret_cast<const Pds::BldInfo&>(src);
    switch(bld.type()) {
    case Pds::BldInfo::HxxDg1Cam:
    case Pds::BldInfo::HfxDg2Cam:
    case Pds::BldInfo::HfxDg3Cam:
    case Pds::BldInfo::XcsDg3Cam:
    case Pds::BldInfo::HfxMonCam:
      client = new Ami::Qt::ImageClient(this, info, channel); break;
    default: 
      printf("Bld type %x not recognized\n", bld.type()); break;
    }
  }
  else if (src.level()==Pds::Level::Event) {
    client = new Ami::Qt::SummaryClient (this, info , channel, name, ConfigureRequest::User); 
  }
  else {
    printf("Ignoring %s [%08x.%08x]\n",qPrintable(name), src.log(), src.phy());
  }

  if (client) {
    if (p) client->load(p);
    _connect_client(client);
  }

  return client;
 }

 void DetectorSelect::_connect_client(Ami::Qt::AbsClient* client)
 {
   ClientManager* manager = new ClientManager(_ppinterface,
                _interface,     
                _serverGroup,
                _clientPort++,
                *client); 
   client->managed(*manager);         
   _client.push_back(client);
   //   _update_groups();

   connect(client, SIGNAL(changed()), this, SLOT(queue_autosave()));
}
   
void DetectorSelect::show_detector(QListWidgetItem* item)
{
  DetectorListItem* ditem = static_cast<DetectorListItem*>(item);
  for(std::list<QtTopWidget*>::iterator it = _client.begin(); it != _client.end(); it++)
    if (match((*it)->info,ditem->info) && (*it)->channel==ditem->channel) {
      (*it)->show();
      return;
    }
  const char* p=0;
  _create_client(ditem->info,ditem->channel,ditem->text(),p);
}

void DetectorSelect::connected       () 
{
  _manager->discover(); 
}

int  DetectorSelect::configure       (iovec* iov) 
{ 
  char* p = _request;
  { ConfigureRequest& r = 
      *new(p) ConfigureRequest(ConfigureRequest::Filter,
                               _filters->selected());
    p += r.size(); }

  _rate_display->configure(p);

  iov[0].iov_base = _request;
  iov[0].iov_len  = p-_request;
  return 1; 
}

int  DetectorSelect::configured      () { return 0; }

void DetectorSelect::discovered      (const DiscoveryRx& rx) 
{ 
  _rate_display->discovered(rx);

  pthread_mutex_lock(&_mutex);
  _discovered = false;
  emit detectors_discovered(reinterpret_cast<const char*>(&rx));
  for (;;) {
    if (_discovered) {
      break;
    }
    pthread_cond_wait(&_condition, &_mutex);
  }
  pthread_mutex_unlock(&_mutex);

  _manager->configure();
}

void DetectorSelect::change_detectors(const char* c)
{
  const DiscoveryRx& rx = *reinterpret_cast<const DiscoveryRx*>(c);

  setUpdatesEnabled(false);

  //  Remove all clients not discovered
  {
    std::list<QtTopWidget*> remove;
    for(std::list<QtTopWidget*>::iterator it = _client.begin(); it != _client.end(); it++) {
      
      bool lFound=false;
      for(const Ami::DescEntry* e = rx.entries(); e < rx.end();
    e = reinterpret_cast<const Ami::DescEntry*>
      (reinterpret_cast<const char*>(e) + e->size()))
  if ((match((*it)->info,e->info()) && (*it)->channel == e->channel()) ||
      (match((*it)->info,envInfo)   && (*it)->channel <= 1)) {
    lFound=true;
    break;
  }
      if (!lFound)
  remove.push_back(*it);
    }
    for(std::list<QtTopWidget*>::iterator it = remove.begin(); it != remove.end(); it++) {
      printf("DetectorSelect::change_detectors removing %s\n",qPrintable((*it)->title()));
      _client.remove(*it);
    }
  }

  //  Register new buttons
  {
    _detList->clear();

    new DetectorListItem(_detList, "Env"    , envInfo, 0);
    new DetectorListItem(_detList, "PostAnalysis", envInfo, 1);
    //    new DetectorListItem(_detList, "Summary", noInfo , 0);

    const Ami::DescEntry* n = 0;
    const Ami::DescEntry* rx_entries = rx.entries();
    const Ami::DescEntry* rx_end = rx.end();
    for (const Ami::DescEntry* e = rx_entries; e < rx_end; e = n) {
      int e_size = e->size();
      n = reinterpret_cast<const Ami::DescEntry*> (reinterpret_cast<const char*>(e) + e_size);
      if (! _quiet) {
        printf("Discovered %s [%08x.%08x] size=%d\n",e->name(),e->info().log(),e->info().phy(),e->size());
      }
      if (e->info().level() == Pds::Level::Control) {
        if (! _quiet) {
          printf("\tSkip.\n");
        }
      } else {
        new DetectorListItem(_detList, e->name(), e->info(), e->channel());
      }
      if (e->size() == 0) {
        printf("Stopped discovering because size is 0 for %s [%08x.%08x]\n",e->name(),e->info().log(),e->info().phy());
        break;
      }
    }
    _detList->sortItems();
  }

  //  Register filters
  _filters->update(rx);

  //  _update_groups();

  setUpdatesEnabled(true);

  // Let DetectorSelect::discovered know that we are done.
  pthread_mutex_lock(&_mutex);
  _discovered = true;
  pthread_cond_signal(&_condition);
  pthread_mutex_unlock(&_mutex);
}

void DetectorSelect::read_description(Socket& s,int len) {
  _rate_display->read_description(s,len);
}

void DetectorSelect::read_payload    (Socket& s,int len) {
  _rate_display->read_payload(s,len);
}

void DetectorSelect::process         () {
  _rate_display->process();
}

void DetectorSelect::queue_autosave()
{
  _autosave_timer->stop();
  _autosave_timer->start(10000);
}

void DetectorSelect::autosave()
{
  static bool warned = false;

  char* buffer = new char[MaxConfigSize];

  int len = get_setup(buffer);

  QString fname = QString("%1/AUTOSAVE.ami").arg(Path::base());

  FILE* o = fopen(qPrintable(fname),"w");
  if (o) {
    fwrite(buffer,len,1,o);
    fclose(o);
    printf("Saved %d bytes to %s\n",len,qPrintable(fname));
  }
  else if (!warned) {
    warned = true;
    QString msg = QString("Error opening %1 : %2").arg(fname).arg(strerror(errno));
    QMessageBox::critical(this,"Save Error",msg);
  }

  delete[] buffer;
}

void DetectorSelect::autoload()
{
  QString fname = QString("%1/AUTOSAVE.ami").arg(Path::base());
  load_setup(qPrintable(fname));
}
