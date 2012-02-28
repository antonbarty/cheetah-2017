#include "EnvPlot.hh"

#include "ami/qt/AxisArray.hh"
#include "ami/qt/PlotFactory.hh"
#include "ami/qt/QtTH1F.hh"
#include "ami/qt/QtChart.hh"
#include "ami/qt/QtProf.hh"
#include "ami/qt/QtScan.hh"
#include "ami/qt/Path.hh"
#include "ami/qt/FeatureRegistry.hh"

#include "ami/data/AbsFilter.hh"
#include "ami/data/FilterFactory.hh"
#include "ami/data/AbsTransform.hh"
#include "ami/data/Cds.hh"
#include "ami/data/ConfigureRequest.hh"
#include "ami/data/DescEntry.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/EntryProf.hh"
#include "ami/data/EntryScan.hh"
#include "ami/data/EntryScalar.hh"
#include "ami/data/EnvPlot.hh"

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

EnvPlot::EnvPlot(QWidget*         parent,
		 const QString&   name,
		 const Ami::AbsFilter&  filter,
		 DescEntry*       desc) :
  QtPlot   (parent, name),
  _filter  (filter.clone()),
  _desc    (desc),
  _output_signature  (0),
  _plot    (0)
{
}

EnvPlot::EnvPlot(QWidget*     parent,
		 const char*& p) :
  QtPlot   (parent),
  _filter  (0),
  _output_signature(0),
  _plot    (0)
{
  XML_iterate_open(p,tag)
    
    if (tag.element == "QtPlot")
      QtPlot::load(p);
    else if (tag.name == "_filter") {
      Ami::FilterFactory factory;
      const char* b = (const char*)QtPersistent::extract_op(p);
      _filter = factory.deserialize(b);
    }
    else if (tag.name == "_desc") {
      DescEntry* desc = (DescEntry*)QtPersistent::extract_op(p);

      printf("EnvPlot desc %p type %d\n",desc, desc->type());

#define CASEENTRY(type) case DescEntry::type: _desc = new Desc##type(*static_cast<Desc##type*>(desc)); break;

      switch(desc->type()) {
        CASEENTRY(TH1F)
          CASEENTRY(Prof)
          CASEENTRY(Scan)
          CASEENTRY(Scalar)
          default: break;
      }
    }

  XML_iterate_close(EnvPlot,tag);
}

EnvPlot::~EnvPlot()
{
  if (_filter  ) delete _filter;
  delete _desc;
  if (_plot    ) delete _plot;
}

void EnvPlot::save(char*& p) const
{
  char* buff = new char[8*1024];
  XML_insert( p, "QtPlot", "self", QtPlot::save(p) );
  XML_insert( p, "AbsFilter", "_filter", QtPersistent::insert(p, buff, (char*)_filter->serialize(buff)-buff) );
  XML_insert( p, "DescEntry", "_desc", QtPersistent::insert(p, _desc, _desc->size()) );
  delete[] buff;
}


void EnvPlot::load(const char*& p)
{
}

void EnvPlot::dump(FILE* f) const { _plot->dump(f); }

#include "ami/data/Entry.hh"
#include "ami/data/DescEntry.hh"

void EnvPlot::setup_payload(Cds& cds)
{
  if (_plot) delete _plot;
    
  Ami::Entry* entry = cds.entry(_output_signature);
  if (entry) {
    edit_xrange(true);

    printf("EnvPlot::setup_payload %s %d\n",
           entry->desc().name(), entry->desc().type());

    switch(entry->desc().type()) {
    case Ami::DescEntry::TH1F: 
      _plot = new QtTH1F(_name,*static_cast<const Ami::EntryTH1F*>(entry),
			 noTransform,noTransform,QColor(0,0,0));
      break;
    case Ami::DescEntry::Scalar:  // create a chart from a scalar
//       { const DescChart& d = *reinterpret_cast<const DescChart*>(_desc);
// 	_plot = new QtChart(_name,*static_cast<const Ami::EntryScalar*>(entry),
// 			    d.pts(),QColor(0,0,0));
// 	break; }
      _plot = new QtChart(_name,*static_cast<const Ami::EntryScalar*>(entry),
			  QColor(0,0,0));
      edit_xrange(false);
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
      printf("EnvPlot type %d not implemented yet\n",entry->desc().type()); 
      return;
    }
    _plot->attach(_frame);
  }
  else if (_output_signature>=0)
    printf("%s output_signature %d not found\n",qPrintable(_name),_output_signature);
}

void EnvPlot::configure(char*& p, unsigned input, unsigned& output)
{
  Ami::EnvPlot op(*_desc);
  
  ConfigureRequest& r = *new (p) ConfigureRequest(ConfigureRequest::Create,
						  ConfigureRequest::Discovery,
						  input,
						  _output_signature = ++output,
						  *_filter, op);
  p += r.size();
}

void EnvPlot::update()
{
  if (_plot) {
    _plot->update();
    emit counts_changed(_plot->normalization());
    emit redraw();
  }
}
