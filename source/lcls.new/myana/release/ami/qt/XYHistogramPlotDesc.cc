#include "ami/qt/XYHistogramPlotDesc.hh"

#include "ami/qt/QtPersistent.hh"
#include "ami/qt/RectangleCursors.hh"
#include "ami/qt/DescTH1F.hh"

#include "ami/data/DescTH1F.hh"
#include "ami/data/XYHistogram.hh"

#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>

using namespace Ami::Qt;

enum { PlotSum, PlotMean };

XYHistogramPlotDesc::XYHistogramPlotDesc(QWidget* parent,
                                         const RectangleCursors& r) :
  QWidget(parent),
  _rectangle(r)
{
  QVBoxLayout* layout1 = new QVBoxLayout;
  layout1->addStretch();
  layout1->addWidget(_desc = new DescTH1F("Pixel Values"));
  layout1->addStretch();
  setLayout(layout1);
}

XYHistogramPlotDesc::~XYHistogramPlotDesc()
{
}

void XYHistogramPlotDesc::save(char*& p) const
{
  XML_insert( p, "DescTH1F", "_desc", _desc->save(p) );
}

void XYHistogramPlotDesc::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if (tag.name == "_desc")
      _desc->load(p);
  XML_iterate_close(XYHistogramPlotDesc,tag);
}

Ami::XYHistogram* XYHistogramPlotDesc::desc(const char* title) const
{
  Ami::DescTH1F desc(title,
                     "pixel value", "pixels",
                     _desc->bins(), _desc->lo(), _desc->hi());
  return new Ami::XYHistogram(desc, 
                              _rectangle.xlo(), _rectangle.xhi(),
                              _rectangle.ylo(), _rectangle.yhi());
}
