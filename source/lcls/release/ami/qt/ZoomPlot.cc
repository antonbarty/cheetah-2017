#include "ZoomPlot.hh"

#include "ami/qt/QtImage.hh"
#include "ami/qt/ImageDisplay.hh"
#include "ami/qt/ImageFrame.hh"
#include "ami/qt/ImageGridScale.hh"
#include "ami/qt/ChannelDefinition.hh"
#include "ami/qt/Filter.hh"

#include "ami/data/Cds.hh"
#include "ami/data/DescImage.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/ConfigureRequest.hh"
#include "ami/data/Zoom.hh"

#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGroupBox>

#include "qwt_plot.h"

using namespace Ami::Qt;

ZoomPlot::ZoomPlot(QWidget*         parent,
		   const QString&   name,
		   unsigned         input_channel,
		   unsigned         x0, 
		   unsigned         y0,
		   unsigned         x1,
		   unsigned         y1) :
  QtPWidget(parent),
  _name    (name),
  _input   (input_channel),
  _signature(-1),
  _x0     (x0),
  _y0     (y0),
  _x1     (x1),
  _y1     (y1),
  _frame   (new ImageDisplay)
{
  setWindowTitle(name);
  setAttribute(::Qt::WA_DeleteOnClose, true);

  QHBoxLayout* layout = new QHBoxLayout;
  layout->addWidget(_frame);
  setLayout(layout);

  show();
}

ZoomPlot::ZoomPlot(QWidget*         parent,
		   const char*&     p) :
  QtPWidget(parent),
  _signature(-1),
  _frame   (new ImageDisplay)
{
  load(p);

  setWindowTitle(_name);
  setAttribute(::Qt::WA_DeleteOnClose, true);

  QHBoxLayout* layout = new QHBoxLayout;
  layout->addWidget(_frame);
  setLayout(layout);

  show();
}

ZoomPlot::~ZoomPlot()
{
  printf("ZoomPlot %p deleted\n",this);
}

void ZoomPlot::save(char*& p) const
{
  XML_insert(p, "QtPWidget", "self", QtPWidget::save(p) );

  XML_insert(p, "QString", "_name", QtPersistent::insert(p,_name) );
  XML_insert(p, "unsigned", "_input", QtPersistent::insert(p,_input) );
  XML_insert(p, "unsigned", "_x0", QtPersistent::insert(p,_x0) );
  XML_insert(p, "unsigned", "_y0", QtPersistent::insert(p,_y0) );
  XML_insert(p, "unsigned", "_x1", QtPersistent::insert(p,_x1) );
  XML_insert(p, "unsigned", "_y1", QtPersistent::insert(p,_y1) );
  XML_insert(p, "ImageDisplay", "_frame", _frame->save(p) );
}

void ZoomPlot::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if (tag.element == "QtPWidget")
      QtPWidget::load(p);
    else if (tag.name == "_name")
      _name  = QtPersistent::extract_s(p);
    else if (tag.name == "_input")
      _input = QtPersistent::extract_i(p);
    else if (tag.name == "_x0")
      _x0 = QtPersistent::extract_i(p);
    else if (tag.name == "_y0")
      _y0 = QtPersistent::extract_i(p);
    else if (tag.name == "_x1")
      _x1 = QtPersistent::extract_i(p);
    else if (tag.name == "_y1")
      _y1 = QtPersistent::extract_i(p);
    else if (tag.name == "_frame")
      _frame->load(p);
  XML_iterate_close(ZoomPlot,tag);
}

void ZoomPlot::setup_payload(Cds& cds)
{
  _frame->reset();
  const Entry* entry = cds.entry(_signature);
  if (entry) {
    _frame->add( new QtImage(entry->desc().name(),
			     *static_cast<const EntryImage*>(entry),
			     _x0, _y0, _x1, _y1),
		 true);
    _frame->grid_scale().setup_payload(cds);
  }
}

void ZoomPlot::configure(char*& p, 
                         unsigned input, 
                         unsigned& output,
                         ChannelDefinition* input_channels[],
                         int* input_signatures, 
                         unsigned input_nchannels)
{
#if 0
  DescImage image(qPrintable(_name),
                  unsigned(_x1-_x0+1), unsigned(_y1-_y0+1),
                    1,   1,
                  _x0, _y0);
  Ami::Zoom zoom(image, input_channels[_input]->oper());

  ConfigureRequest& r = *new (p) ConfigureRequest(ConfigureRequest::Create,
						  ConfigureRequest::Hidden,
						  input,
						  _signature = ++output,
						  *input_channels[_input]->filter().filter(),
						  zoom);
  p += r.size();
#else
  _signature = input_signatures[_input];
#endif
}

void ZoomPlot::update()
{
  _frame  ->update();
}
