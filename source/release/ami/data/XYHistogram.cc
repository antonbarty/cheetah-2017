#include "XYHistogram.hh"

#include "ami/data/DescEntry.hh"
#include "ami/data/DescTH1F.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/EntryFactory.hh"

#include "ami/data/Cds.hh"

#include <stdio.h>

using namespace Ami;

XYHistogram::XYHistogram(const DescEntry& output, 
                         double xlo, double xhi,
                         double ylo, double yhi) :
  AbsOperator(AbsOperator::XYHistogram),
  _xlo        (xlo),
  _xhi        (xhi),
  _ylo        (ylo),
  _yhi        (yhi),
  _output    (0)
{
  memcpy(_desc_buffer, &output, output.size());
}

XYHistogram::XYHistogram(const char*& p, const DescEntry& input) :
  AbsOperator(AbsOperator::XYHistogram)
{
  _extract(p, _desc_buffer, DESC_LEN);
  _extract(p, &_xlo        , sizeof(_xlo ));
  _extract(p, &_xhi        , sizeof(_xhi ));
  _extract(p, &_ylo        , sizeof(_ylo ));
  _extract(p, &_yhi        , sizeof(_yhi ));

  const DescEntry& o = *reinterpret_cast<const DescEntry*>(_desc_buffer);
  _output = EntryFactory::entry(o);
}

XYHistogram::XYHistogram(const char*& p) :
  AbsOperator(AbsOperator::XYHistogram),
  _output    (0)
{
  _extract(p, _desc_buffer, DESC_LEN);
  _extract(p, &_xlo        , sizeof(_xlo ));
  _extract(p, &_xhi        , sizeof(_xhi ));
  _extract(p, &_ylo        , sizeof(_ylo ));
  _extract(p, &_yhi        , sizeof(_yhi ));
}

XYHistogram::~XYHistogram()
{
  if (_output) delete _output;
}

DescEntry& XYHistogram::output   () const 
{ 
  return _output ? _output->desc() : *reinterpret_cast<DescEntry*>(const_cast<char*>(_desc_buffer)); 
}

void*      XYHistogram::_serialize(void* p) const
{
  _insert(p, _desc_buffer, DESC_LEN);
  _insert(p, &_xlo        , sizeof(_xlo ));
  _insert(p, &_xhi        , sizeof(_xhi ));
  _insert(p, &_ylo        , sizeof(_ylo ));
  _insert(p, &_yhi        , sizeof(_yhi ));
  return p;
}

Entry&     XYHistogram::_operate(const Entry& e) const
{
  if (!e.valid())
    return *_output;

  const EntryImage* _input = static_cast<const EntryImage*>(&e);
  const DescImage& inputd = _input->desc();
  if (_input) {
    switch(output().type()) {
    case DescEntry::TH1F:  // unnormalized
      { EntryTH1F*      o = static_cast<EntryTH1F*>(_output);
        o->clear();

        unsigned ilo = unsigned((_xlo-inputd.xlow())/inputd.ppxbin());
        unsigned ihi = unsigned((_xhi-inputd.xlow())/inputd.ppxbin());
        unsigned jlo = unsigned((_ylo-inputd.ylow())/inputd.ppybin());
        unsigned jhi = unsigned((_yhi-inputd.ylow())/inputd.ppybin());
        double   p(_input->info(EntryImage::Pedestal));
        double   n   = 1./double(inputd.ppxbin()*inputd.ppybin());
        for(unsigned i=ilo; i<ihi; i++) {
          for(unsigned j=jlo; j<jhi; j++)
            o->addcontent(1.,(double(_input->content(i,j))-p)*n);
	}
	o->info(_input->info(EntryImage::Normalization),EntryTH1F::Normalization);
	break; }
    default:
      break;
    }
  }
  _output->valid(e.time());
  return *_output;
}
