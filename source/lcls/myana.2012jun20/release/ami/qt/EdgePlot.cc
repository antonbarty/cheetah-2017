#include "EdgePlot.hh"

#include "ami/qt/AxisInfo.hh"
#include "ami/qt/ChannelDefinition.hh"
#include "ami/qt/Filter.hh"
#include "ami/qt/PlotFactory.hh"
#include "ami/qt/QtBase.hh"

#include "ami/data/Cds.hh"
#include "ami/data/ConfigureRequest.hh"
#include "ami/data/AbsTransform.hh"
#include "ami/data/DescEntry.hh"
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
  _fcnt    (0)
{
  for (int i = 0; i < MAX_FINDERS; i++) {
    _finder[i] = 0;
    _plot[i] = 0;
  }
  addfinder(finder);
}

EdgePlot::EdgePlot(QWidget* parent,
		   const char*& p) :
  QtPlot   (parent),
  _fcnt    (0)
{
  for (int i = 0; i < MAX_FINDERS; i++) {
    _finder[i] = 0;
    _plot[i] = 0;
  }
  XML_iterate_open(p,tag)
    if (tag.element == "QtPlot")
      QtPlot::load(p);
    else if (tag.name == "_channel")
      _channel = QtPersistent::extract_i(p);
    else if (tag.name == "_finder") {
      const char* b = (const char*)QtPersistent::extract_op(p);
      b += 2*sizeof(uint32_t);
      _finder[0] = new Ami::EdgeFinder(0.5,
                                       Ami::EdgeFinder::EdgeAlgorithm(Ami::EdgeFinder::halfbase2peak,
                                                                      true), 0.0, b);
      _fcnt = 1;
    } else if (tag.name.compare(0,7,"_finder") == 0) { /* _finderNNN */
        int i = atoi(tag.name.substr(7).c_str());
        Ami::EdgeFinder *f = loadfinder(p);
        if (i < MAX_FINDERS) {
            _finder[i] = f;
            if (i + 1 > _fcnt)
                _fcnt = i + 1;
        } else {
            printf("EdgePlot: unknown tag %s/%s\n", tag.element.c_str(), tag.name.c_str());
            delete f;
        }
    }
  XML_iterate_close(EdgePlot,tag);
}

EdgePlot::~EdgePlot()
{
  for (int i = 0; i < _fcnt; i++) {
    if (_finder[i]  ) delete _finder[i];
    if (_plot[i]    ) delete _plot[i];
  }
}

void EdgePlot::savefinder(Ami::EdgeFinder *f, char*& p) const
{
    XML_insert( p, "double",   "_threshold", QtPersistent::insert(p,f->threshold()) );
    XML_insert( p, "double",   "_baseline",  QtPersistent::insert(p,f->baseline()) );
    XML_insert( p, "int",      "_algorithm", QtPersistent::insert(p,f->algorithm()) );
    XML_insert( p, "double",   "_fraction",  QtPersistent::insert(p,f->fraction()) );
    XML_insert( p, "double",   "_deadtime",  QtPersistent::insert(p,f->deadtime()) );
    XML_insert( p, "DescEntry","_output",    QtPersistent::insert(p,f->desc(), f->desc_size()));
    XML_insert( p, "Parameter","_parameter", QtPersistent::insert(p,f->parameter()));
}

Ami::EdgeFinder *EdgePlot::loadfinder(const char*& p) 
{
  double thresh = 0.0, base = 0.0;
  int alg = Ami::EdgeFinder::EdgeAlgorithm(Ami::EdgeFinder::halfbase2peak, true);
  double deadtime = 0.0;
  double fraction = 0.5;
  const Ami::DescEntry *desc = NULL;
  EdgeFinder::Parameter parm = EdgeFinder::Location;

  XML_iterate_open(p,tag)
    if (tag.name == "_threshold")
      thresh = Ami::Qt::QtPersistent::extract_d(p);
    else if (tag.name == "_baseline")
      base = Ami::Qt::QtPersistent::extract_d(p);
    else if (tag.name == "_algorithm")
      alg = Ami::Qt::QtPersistent::extract_i(p);
    else if (tag.name == "_deadtime")
      deadtime = Ami::Qt::QtPersistent::extract_d(p);
    else if (tag.name == "_fraction")
      fraction = Ami::Qt::QtPersistent::extract_d(p);
    else if (tag.name == "_output") {
      desc = (const Ami::DescEntry*)QtPersistent::extract_op(p);
    }
    else if (tag.name == "_parameter")
      parm = Ami::EdgeFinder::Parameter(Ami::Qt::QtPersistent::extract_i(p));
  XML_iterate_close(EdgeFinder,tag);

  if (desc)
    return new Ami::EdgeFinder(fraction, thresh, base, alg, deadtime, *desc, parm);
  else
      return NULL;
}

void EdgePlot::save(char*& p) const
{
  char* buff = new char[8*1024];

  XML_insert( p, "QtPlot", "self",
              QtPlot::save(p) );

  XML_insert( p, "int", "_channel",
              QtPersistent::insert(p,(int)_channel) );

  /*
   * We used to save a single binary EdgeFinder with the following command:
   *   XML_insert( p, "EdgeFinder", "_finder",
   *              QtPersistent::insert(p, buff, (char*)_finder[0]->serialize(buff)-buff) );
   * load() will still support this.
   */
  for (int i = 0; i < _fcnt; i++) {
      sprintf(buff, "_finder%d", i);
      XML_insert(p, "EdgeFinder", buff, savefinder(_finder[i], p));
  }

  delete[] buff;
}

void EdgePlot::load(const char*& p) 
{
}

void EdgePlot::dump(FILE* f)          const { _plot[0]->dump(f); }
void EdgePlot::dump(FILE* f, int idx) const { _plot[idx]->dump(f); }

#include "ami/data/Entry.hh"
#include "ami/data/DescEntry.hh"

/*
 * This gives the color for the various plots.
 *
 * For now, the first will be black, and the second (and subsequent) will be red.
 */
QColor &EdgePlot::getcolor(int i)
{
  static QColor c;
  c.setRgb(i ? 0xff : 0, 0, 0);
  return c;
}

void EdgePlot::setup_payload(Cds& cds)
{
  Ami::Entry* entry;
  for (int i = 0; i < _fcnt; i++) {
    if (_plot[i]) {
      delete _plot[i];
      _plot[i] = 0;
    };
    entry  = cds.entry(_output_signature + i);
    if (entry) {
      _plot[i] = PlotFactory::plot(_name,*entry,
                                   noTransform,noTransform,getcolor(i));
      _plot[i]->attach(_frame);
    }
    else if (_output_signature + i >=0)
      printf("%s output_signature %d not found\n",qPrintable(_name), _output_signature + i);
  }
}

void EdgePlot::addfinder(Ami::EdgeFinder *f)
{
  _finder[_fcnt++] = f;
}

void EdgePlot::configure(char*& p, unsigned input, unsigned& output,
			   ChannelDefinition* channels[], int* signatures, unsigned nchannels,
			   const AxisInfo& xinfo)
{
  unsigned input_signature = signatures[_channel];

  ConfigureRequest *r;
  _output_signature = output + 1;
  for (int i = 0; i < _fcnt; i++) {
    r = new (p) ConfigureRequest(ConfigureRequest::Create,
                                 ConfigureRequest::Analysis,
                                 input_signature,
                                 ++output,
                                 *channels[_channel]->filter().filter(),
                                 *_finder[i]);
    p += r->size();
  }
}

void EdgePlot::update()
{
  bool have_change = false;
  for (int i = 0; i < _fcnt; i++) {
      if (_plot[i]) {
          _plot[i]->update();
          emit counts_changed(_plot[0]->normalization());
          have_change = true;
      }
  }
  if (have_change)
    emit redraw();
}
