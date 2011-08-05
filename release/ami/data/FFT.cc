#include "FFT.hh"

#include "ami/data/Complex.hh"
#include "ami/data/DescEntry.hh"
#include "ami/data/DescTH1F.hh"
#include "ami/data/DescProf.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/EntryProf.hh"
#include "ami/data/EntryFactory.hh"

#include "ami/data/Cds.hh"

#include <stdio.h>
#include <math.h>

using namespace Ami;

FFT::FFT() : 
  AbsOperator(AbsOperator::FFT),
  _output    (0) 
{
}

FFT::FFT(const char*& p, const DescEntry& input) :
  AbsOperator(AbsOperator::FFT)
{
  switch(input.type()) {
  case DescEntry::TH1F:
    { const DescTH1F& inputd = static_cast<const DescTH1F&>(input);
      float w = 0.5*float(inputd.nbins()-1)/(inputd.xup()-inputd.xlow());
      unsigned n = inputd.nbins()>>1;
      DescTH1F desc(inputd.info(), inputd.channel(),
		    inputd.name(), inputd.xtitle(), inputd.ytitle(),
		    n, 0., w*float(n),false);
      _output = new EntryTH1F(desc);
      break; }
  case DescEntry::Prof:
    { const DescProf& inputd = static_cast<const DescProf&>(input);
      float w = 0.5*float(inputd.nbins()-1)/(inputd.xup()-inputd.xlow());
      unsigned n = inputd.nbins()>>1;
      DescProf desc(inputd.info(), inputd.channel(),
		    inputd.name(), inputd.xtitle(), inputd.ytitle(),
		    n, 0., w*float(n), NULL);
      _output = new EntryProf(desc);
      break; }
  default:
    _output = 0;
    printf("FFT of type %d not supported\n",input.type());
    break;
  }
}

FFT::~FFT()
{
  if (_output) delete _output;
}

DescEntry& FFT::output   () const 
{ 
  return _output->desc();
}

void*      FFT::_serialize(void* p) const
{
  return p;
}

Entry&     FFT::_operate(const Entry& e) const
{
  switch(output().type()) {
  case DescEntry::TH1F:  // unnormalized
  case DescEntry::Prof:  // normalized
  default:
    break;
  }
  return *_output;
}
