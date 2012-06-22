#include "CurveFitPlot.hh"

#include "ami/qt/AxisInfo.hh"
#include "ami/qt/ChannelDefinition.hh"
#include "ami/qt/Filter.hh"
#include "ami/qt/PlotFactory.hh"
#include "ami/qt/QtTH1F.hh"
#include "ami/qt/QtChart.hh"
#include "ami/qt/QtProf.hh"
#include "ami/qt/QtScan.hh"

#include "ami/data/Cds.hh"
#include "ami/data/ConfigureRequest.hh"
#include "ami/data/AbsTransform.hh"
#include "ami/data/DescEntry.hh"
#include "ami/data/CurveFit.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/EntryProf.hh"
#include "ami/data/EntryScan.hh"
#include "ami/data/EntryScalar.hh"

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

CurveFitPlot::CurveFitPlot(QWidget*         parent,
		           const QString&   name,
		           unsigned         channel,
		           Ami::CurveFit* fit) :
  QtPlot   (parent, name),
  _channel (channel),
  _fit     (fit),
  _output_signature (0),
  _plot    (0)
{
}

CurveFitPlot::CurveFitPlot(QWidget* parent,
		   const char*& p) :
  QtPlot   (parent),
  _fit     (0),
  _plot    (0)
{
  load(p);
}

CurveFitPlot::~CurveFitPlot()
{
  delete _fit;
  if (_plot    ) delete _plot;
}

void CurveFitPlot::save(char*& p) const
{
  char* buff = new char[8*1024];

  XML_insert( p, "QtPlot", "self",
              QtPlot::save(p) );

  XML_insert( p, "int", "_channel",
              QtPersistent::insert(p,(int)_channel) );

  XML_insert( p, "CurveFit", "_fit", savefit(p));

  delete[] buff;
}

void CurveFitPlot::savefit(char*& p) const
{
    DescEntry &desc = _fit->output();
    XML_insert( p, "QString"     , "_name"  , QtPersistent::insert(p, QString(_fit->name())));
    XML_insert( p, "QString"     , "_norm"  , QtPersistent::insert(p, QString(_fit->norm())));
    XML_insert( p, "int"         , "_op"    , QtPersistent::insert(p, _fit->op()) );
    XML_insert( p, "DescEntry"   , "_output", QtPersistent::insert(p, &desc, desc.size()));
}

Ami::CurveFit *CurveFitPlot::loadfit(const char*& p)
{
  QString name;
  QString norm;
  int op = 0;
  DescEntry *desc = NULL;

  name.clear();
  XML_iterate_open(p,tag)
      if (tag.name == "_name")
          name = QtPersistent::extract_s(p);
      else if (tag.name == "_norm")
          norm = QtPersistent::extract_s(p);
      else if (tag.name == "_op")
          op = Ami::Qt::QtPersistent::extract_i(p);
      else if (tag.name == "_output")
          desc = (DescEntry*)QtPersistent::extract_op(p);
  XML_iterate_close(CurveFit,tag);

  if (desc)
      return new Ami::CurveFit(qPrintable(name), op, *desc, qPrintable(norm));
  else
      return NULL;
}

void CurveFitPlot::load(const char*& p) 
{
  XML_iterate_open(p,tag)
    if (tag.element == "QtPlot")
      QtPlot::load(p);
    else if (tag.name == "_channel")
      _channel = QtPersistent::extract_i(p);
    else if (tag.name == "_fit") {
      _fit = loadfit(p);
    }
  XML_iterate_close(CurveFitPlot,tag);
}

void CurveFitPlot::dump(FILE* f) const { _plot->dump(f); }

#include "ami/data/Entry.hh"
#include "ami/data/DescEntry.hh"

void CurveFitPlot::setup_payload(Cds& cds)
{
  if (_plot) {
    delete _plot;
    _plot = 0;
  }
    
  Ami::Entry* entry = cds.entry(_output_signature);
  if (entry) {
    switch(entry->desc().type()) {
    case Ami::DescEntry::TH1F: 
      _plot = new QtTH1F(_name,*static_cast<const Ami::EntryTH1F*>(entry),
			 noTransform,noTransform,QColor(0,0,0));
      break;
    case Ami::DescEntry::Scalar:  // create a chart from a scalar
      _plot = new QtChart(_name,*static_cast<const Ami::EntryScalar*>(entry),
			  QColor(0,0,0));
      break;
    case Ami::DescEntry::Prof: 
      _plot = new QtProf(_name,*static_cast<const Ami::EntryProf*>(entry),
			 noTransform,noTransform,QColor(0,0,0));
      break;
    case Ami::DescEntry::Scan: 
      _plot = new QtScan(_name,*static_cast<const Ami::EntryScan*>(entry),
			 noTransform,noTransform,QColor(0,0,0));
      break;
    default:
      printf("CurveFitPlot type %d not implemented yet\n",entry->desc().type()); 
      return;
    }
    _plot->attach(_frame);
  }
  else if (_output_signature>=0)
    printf("%s output_signature %d not found\n",qPrintable(_name),_output_signature);
}

void CurveFitPlot::configure(char*& p, unsigned input, unsigned& output,
			   ChannelDefinition* channels[], int* signatures, unsigned nchannels,
			   const AxisInfo& xinfo)
{
  unsigned input_signature = signatures[_channel];

  ConfigureRequest& r = *new (p) ConfigureRequest(ConfigureRequest::Create,
						  ConfigureRequest::Analysis,
						  input_signature,
						  _output_signature = ++output,
						  *channels[_channel]->filter().filter(),
						  *_fit);
  p += r.size();
}

void CurveFitPlot::update()
{
  if (_plot) {
    _plot->update();
    emit counts_changed(_plot->normalization());
    emit redraw();
  }
}
