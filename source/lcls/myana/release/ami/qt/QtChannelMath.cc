#include "ami/qt/QtChannelMath.hh"

using namespace Ami::Qt;

QtChannelMath::QtChannelMath(QtBase* base, 
			     const std::list<ChannelRef*>& inputs, Term* t) :
  QtBase (base->title(),base->entry()),
  _base  (base),
  _inputs(inputs),
  _term  (t)
{
}

QtChannelMath::~QtChannelMath()
{
  delete _base;
}

void        QtChannelMath::dump  (FILE*) const
{
}

void        QtChannelMath::attach(QwtPlot* p)
{
  _base->attach(p);
}

void        QtChannelMath::update()        
{
  // how to iterate through the payload and apply the math
}

void        QtChannelMath::xscale_update() 
{
  _base->xscale_update();
}

void        QtChannelMath::yscale_update() 
{
  _base->yscale_update();
}
