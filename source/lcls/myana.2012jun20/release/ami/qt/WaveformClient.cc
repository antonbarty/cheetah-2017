#include "WaveformClient.hh"

#include "ami/qt/WaveformDisplay.hh"
#include "ami/qt/EdgeFinder.hh"
#include "ami/qt/CursorsX.hh"
#include "ami/qt/CurveFit.hh"

#include <QtGui/QPushButton>

using namespace Ami::Qt;

WaveformClient::WaveformClient(QWidget* parent,const Pds::DetInfo& info, unsigned ch) :
  Client  (parent,info,ch, new WaveformDisplay),
  _initialized(false)
{
  WaveformDisplay& wd = static_cast<WaveformDisplay&>(display());

  { QPushButton* edgesB = new QPushButton("Edges");
    addWidget(edgesB);
    _edges = new EdgeFinder(this,_channels,NCHANNELS,wd);
    connect(edgesB, SIGNAL(clicked()), _edges, SLOT(show())); }
  { QPushButton* cursorsB = new QPushButton("Cursors");
    addWidget(cursorsB);
    _cursors = new CursorsX(this,_channels,NCHANNELS,wd);
    connect(cursorsB, SIGNAL(clicked()), _cursors, SLOT(show())); }
  { QPushButton* fitB = new QPushButton("Waveform Fit");
    addWidget(fitB);
    _fits = new CurveFit(this,_channels,NCHANNELS,wd);
    connect(fitB, SIGNAL(clicked()), _fits, SLOT(show())); }

  connect(_cursors, SIGNAL(changed()), this, SLOT(update_configuration()));
  connect(_edges  , SIGNAL(changed()), this, SLOT(update_configuration()));
  connect(_fits   , SIGNAL(changed()), this, SLOT(update_configuration()));
}

WaveformClient::~WaveformClient() {}

void WaveformClient::save(char*& p) const
{
  XML_insert(p, "Client", "self", Client::save(p) );
  
  XML_insert(p, "EdgeFinder", "_edges", _edges  ->save(p) );
  XML_insert(p, "CursorsX", "_cursors", _cursors->save(p) );
  XML_insert(p, "CurveFit", "_fits",    _fits->save(p) );
}

void WaveformClient::load(const char*& p)
{
  _initialized = true;

  disconnect(_cursors, SIGNAL(changed()), this, SLOT(update_configuration()));
  disconnect(_edges  , SIGNAL(changed()), this, SLOT(update_configuration()));
  disconnect(_fits   , SIGNAL(changed()), this, SLOT(update_configuration()));
  
  XML_iterate_open(p,tag)

    if (tag.element == "Client")
      Client::load(p);
    else if (tag.name == "_edges")
      _edges  ->load(p);
    else if (tag.name == "_cursors")
      _cursors->load(p);
    else if (tag.name == "_fits")
      _fits->load(p);

  XML_iterate_close(WaveformClient,tag);

  connect(_cursors, SIGNAL(changed()), this, SLOT(update_configuration()));
  connect(_edges  , SIGNAL(changed()), this, SLOT(update_configuration()));
  connect(_fits   , SIGNAL(changed()), this, SLOT(update_configuration()));

  update_configuration();
}

void WaveformClient::save_plots(const QString& p) const
{
  const WaveformDisplay& wd = static_cast<const WaveformDisplay&>(display());
  wd.save_plots(p);
  _edges  ->save_plots(p+"_edge");
  _cursors->save_plots(p+"_cursor");
  _fits   ->save_plots(p+"_fit");
}

void WaveformClient::_prototype(const DescEntry& e)
{
  if (!_initialized) {
    _initialized = true;
    
    _edges  ->initialize(e);
    _cursors->initialize(e);
    _fits   ->initialize(e);
  }
}

void WaveformClient::_configure(char*& p, 
				unsigned input, 
				unsigned& output,
				ChannelDefinition* ch[], 
				int* signatures, 
				unsigned nchannels)
{
  _edges  ->configure(p, input, output,
		      ch, signatures, nchannels);
  _cursors->configure(p, input, output,
		      ch, signatures, nchannels,
		      ConfigureRequest::Analysis);
  _fits   ->configure(p, input, output,
		      ch, signatures, nchannels);
}

void WaveformClient::_setup_payload(Cds& cds)
{
  _edges  ->setup_payload(cds);
  _cursors->setup_payload(cds);
  _fits   ->setup_payload(cds);
}

void WaveformClient::_update()
{
  _edges  ->update();
  _cursors->update();
  _fits   ->update();
}
