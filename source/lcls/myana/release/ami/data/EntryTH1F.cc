#include "ami/data/EntryTH1F.hh"

static const unsigned DefaultNbins = 1;
static const float DefaultLo = 0;
static const float DefaultUp = 1;

using namespace Ami;

#define SIZE(nb) (nb+InfoSize)

EntryTH1F::~EntryTH1F() {}

EntryTH1F::EntryTH1F(const Pds::DetInfo& info, unsigned channel,
		     const char* name, 
		     const char* xtitle, 
		     const char* ytitle) :
  _desc(info, channel, name, xtitle, ytitle, DefaultNbins, DefaultLo, DefaultUp)
{
  build(DefaultNbins);
}

EntryTH1F::EntryTH1F(const DescTH1F& desc) :
  _desc(desc)
{
  build(desc.nbins());
}

void EntryTH1F::params(unsigned nbins, float xlow, float xup)
{
  _desc.params(nbins, xlow, xup);
  build(nbins);
}

void EntryTH1F::params(const DescTH1F& desc)
{
  _desc = desc;
  build(_desc.nbins());
}

void EntryTH1F::clear()
{
  double* dst = _y;
  const double* end = dst+SIZE(_desc.nbins());
  do {
    *dst++ = 0;
  } while (dst < end);
}

void EntryTH1F::build(unsigned nbins)
{
  _y = static_cast<double*>(allocate(sizeof(double)*SIZE(nbins)));
}

const DescTH1F& EntryTH1F::desc() const {return _desc;}
DescTH1F& EntryTH1F::desc() {return _desc;}

void EntryTH1F::setto(const EntryTH1F& entry) 
{
  double* dst = _y;
  const double* end = dst+SIZE(_desc.nbins());
  const double* src = entry._y;
  do {
    *dst++ = *src++;
  } while (dst < end);
  valid(entry.time());
}

void EntryTH1F::setto(const EntryTH1F& curr, 
		      const EntryTH1F& prev) 
{
  double* dst = _y;
  const double* end = dst+SIZE(_desc.nbins());
  const double* srccurr = curr._y;
  const double* srcprev = prev._y;
  do {
    *dst++ = *srccurr++ - *srcprev++;
  } while (dst < end);
  valid(curr.time());
}

void EntryTH1F::add(const EntryTH1F& entry)
{
  double* dst = _y;
  const double* end = dst+SIZE(_desc.nbins());
  const double* src = entry._y;
  do {
    *dst++ += *src++;
  } while (dst < end);
}

void EntryTH1F::addcontent(double y, double x)
{
  if (x < _desc.xlow()) 
    addinfo(y, Underflow);
  else if (x >= _desc.xup()) 
    addinfo(y, Overflow);
  else {
    unsigned bin = unsigned(((x-_desc.xlow())*double(_desc.nbins()) / 
			     (_desc.xup()-_desc.xlow())));
    addcontent(y,bin);
  }
}

