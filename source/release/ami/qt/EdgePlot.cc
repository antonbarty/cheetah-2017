#include "EdgePlot.hh"

#include "ami/qt/AxisInfo.hh"
#include "ami/qt/ChannelDefinition.hh"
#include "ami/qt/Filter.hh"
#include "ami/qt/PlotFactory.hh"
#include "ami/qt/QtTH1F.hh"

#include "ami/data/Cds.hh"
#include "ami/data/ConfigureRequest.hh"
#include "ami/data/AbsTransform.hh"
#include "ami/data/DescEntry.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/EdgeFinder.hh"

#include <QtGui/QLabel>
#include "qwt_plot.h"

namespace Ami {
  namespace Qt {
    class NullTransform : public Ami::AbsTransform {
    public:
      ~NullTransform() {}
      double operator()(double x) const { return x; }
    };
  };
};

using namespace Ami::Qt;

static NullTransform noTransform;

EdgePlot::EdgePlot(QWidget*         parent,
		   const QString&   name,
		   unsigned         channel,
		   Ami::EdgeFinder* finder) :
  QtPlot   (parent, name),
  _channel (channel),
  _finder  (finder),
  _plot    (0)
{
}

EdgePlot::EdgePlot(QWidget* parent,
		   const char*& p) :
  QtPlot   (parent),
  _finder  (0),
  _plot    (0)
{
  XML_iterate_open(p,tag)
    if (tag.element == "QtPlot")
      QtPlot::load(p);
    else if (tag.name == "_channel")
      _channel = QtPersistent::extract_i(p);
    else if (tag.name == "_finder") {
      const char* b = (const char*)QtPersistent::extract_op(p);
      b += 2*sizeof(uint32_t);
      _finder = new Ami::EdgeFinder(b);
    }
  XML_iterate_close(EdgePlot,tag);
}

EdgePlot::~EdgePlot()
{
  delete _finder;
  if (_plot    ) delete _plot;
}

void EdgePlot::save(char*& p) const
{
  char* buff = new char[8*1024];

  XML_insert( p, "QtPlot", "self",
              QtPlot::save(p) );

  XML_insert( p, "int", "_channel",
              QtPersistent::insert(p,(int)_channel) );

  XML_insert( p, "EdgeFinder", "_finder",
              QtPersistent::insert(p, buff, (char*)_finder->serialize(buff)-buff) );

  delete[] buff;
}

void EdgePlot::load(const char*& p) 
{
}

void EdgePlot::dump(FILE* f) const { _plot->dump(f); }

#include "ami/data/Entry.hh"
#include "ami/data/DescEntry.hh"

void EdgePlot::setup_payload(Cds& cds)
{
  if (_plot) delete _plot;
    
  Ami::Entry* entry = cds.entry(_output_signature);
  if (entry) {
    _plot = new QtTH1F(_name,*static_cast<const Ami::EntryTH1F*>(entry),
		       noTransform,noTransform,QColor(0,0,0));
    _plot->attach(_frame);
  }
  else if (_output_signature>=0)
    printf("%s output_signature %d not found\n",qPrintable(_name),_output_signature);
}

void EdgePlot::configure(char*& p, unsigned input, unsigned& output,
			   ChannelDefinition* channels[], int* signatures, unsigned nchannels,
			   const AxisInfo& xinfo)
{
  unsigned input_signature = signatures[_channel];

  ConfigureRequest& r = *new (p) ConfigureRequest(ConfigureRequest::Create,
						  ConfigureRequest::Analysis,
						  input_signature,
						  _output_signature = ++output,
						  *channels[_channel]->filter().filter(),
						  *_finder);
  p += r.size();
}

void EdgePlot::update()
{
  if (_plot) {
    _plot->update();
    emit counts_changed(_plot->normalization());
    emit redraw();
  }
}
