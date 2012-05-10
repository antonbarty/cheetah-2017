#include "TdcPlot.hh"

#include "ami/qt/AxisArray.hh"
#include "ami/qt/PlotFactory.hh"
#include "ami/qt/QtBase.hh"
#include "ami/qt/Path.hh"

#include "ami/data/AbsFilter.hh"
#include "ami/data/FilterFactory.hh"
#include "ami/data/AbsTransform.hh"
#include "ami/data/Cds.hh"
#include "ami/data/ConfigureRequest.hh"
#include "ami/data/DescEntry.hh"
#include "ami/data/EntryTH2F.hh"
#include "ami/data/TdcPlot.hh"
#include "ami/data/RawFilter.hh"

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

TdcPlot::TdcPlot(QWidget*         parent,
		 const QString&   name,
		 const Ami::AbsFilter&  filter,
		 DescEntry*       desc,
		 const QString&   expr) :
  QtPlot   (parent, name),
  _filter  (filter.clone()),
  _desc    (desc),
  _expr    (expr),
  _output_signature  (0),
  _plot    (0)
{
}

TdcPlot::TdcPlot(QWidget*     parent,
		 const char*& p) :
  QtPlot   (parent),
  _filter  (0),
  _output_signature(0),
  _plot    (0)
{
  load(p);
}

TdcPlot::~TdcPlot()
{
  if (_filter  ) delete _filter;
  delete _desc;
  if (_plot    ) delete _plot;
}

void TdcPlot::save(char*& p) const
{
  XML_insert(p, "QtPlot", "self", QtPlot::save(p) );
  //  _filter->serialize(p);
  XML_insert(p, "DescEntry", "_desc", QtPersistent::insert(p, _desc, _desc->size()) );
  XML_insert(p, "QString", "_expr", QtPersistent::insert(p,_expr) );
}


void TdcPlot::load(const char*& p)
{
  //  Ami::FilterFactory factory(FeatureRegistry::instance());
  //  _filter = factory.deserialize(p);

  XML_iterate_open(p,tag)
    if (tag.element == "QtPlot")
      QtPlot::load(p);
    else if (tag.name == "_desc") {
      const DescEntry& desc = *reinterpret_cast<DescEntry*>(QtPersistent::extract_op(p));
      switch(desc.type()) {
      case DescEntry::TH2F: _desc = new DescTH2F(static_cast<const DescTH2F&>(desc)); break;
      default             : _desc = 0; printf("TdcPlot::load error desc type %d\n",desc.type()); break;
      }
    }
    else if (tag.name == "_expr")
      _expr = QtPersistent::extract_s(p);
  XML_iterate_close(TdcPlot,tag);
}

void TdcPlot::dump(FILE* f) const { _plot->dump(f); }

#include "ami/data/Entry.hh"
#include "ami/data/DescEntry.hh"

void TdcPlot::setup_payload(Cds& cds)
{
  if (_plot) delete _plot;
  _plot = 0;

  Ami::Entry* entry = cds.entry(_output_signature);
  if (entry) {
    edit_xrange(true);
    _plot = PlotFactory::plot(_name,*entry,
                              noTransform,noTransform,QColor(0,0,0));
    _plot->attach(_frame);
  }
  else if (_output_signature>=0)
    printf("%s output_signature %d not found\n",qPrintable(_name),_output_signature);
}

void TdcPlot::configure(char*& p, unsigned input, unsigned& output)
{
  Ami::TdcPlot op(*_desc,qPrintable(_expr));
  
  ConfigureRequest& r = *new (p) ConfigureRequest(ConfigureRequest::Create,
						  ConfigureRequest::Discovery,
						  input,
						  _output_signature = ++output,
						  RawFilter(), op);
  p += r.size();
}

void TdcPlot::update()
{
  if (_plot) {
    _plot->update();
    emit counts_changed(_plot->normalization());
    emit redraw();
  }
}
