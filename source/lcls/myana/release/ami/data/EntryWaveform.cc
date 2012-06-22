#include "ami/data/EntryWaveform.hh"

static const unsigned DefaultNbins = 1;
static const float DefaultLo = 0;
static const float DefaultUp = 1;

using namespace Ami;

#define SIZE(nb) (nb+InfoSize)

EntryWaveform::~EntryWaveform() {}

EntryWaveform::EntryWaveform(const Pds::DetInfo& info, unsigned channel,
			     const char* name, 
			     const char* xtitle, 
			     const char* ytitle) :
  _desc(info, channel, name, xtitle, ytitle, DefaultNbins, DefaultLo, DefaultUp)
{
  build(DefaultNbins);
}

EntryWaveform::EntryWaveform(const DescWaveform& desc) :
  _desc(desc)
{
  build(desc.nbins());
}

void EntryWaveform::params(unsigned nbins, float xlow, float xup)
{
  _desc.params(nbins, xlow, xup);
  build(nbins);
}

void EntryWaveform::params(const DescWaveform& desc)
{
  _desc = desc;
  build(_desc.nbins());
}

void EntryWaveform::build(unsigned nbins)
{
  _y = static_cast<double*>(allocate(sizeof(double)*SIZE(nbins)));
}

const DescWaveform& EntryWaveform::desc() const {return _desc;}
DescWaveform& EntryWaveform::desc() {return _desc;}

void EntryWaveform::setto(const EntryWaveform& entry) 
{
  double* dst = _y;
  const double* end = dst+SIZE(_desc.nbins());
  const double* src = entry._y;
  do {
    *dst++ = *src++;
  } while (dst < end);
  valid(entry.time());
}

void EntryWaveform::setto(const EntryWaveform& curr, 
			 const EntryWaveform& prev) 
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

void EntryWaveform::add  (const EntryWaveform& entry) 
{
  double* dst = _y;
  const double* end = dst+SIZE(_desc.nbins());
  const double* src = entry._y;
  do {
    *dst++ += *src++;
  } while (dst < end);
}

void EntryWaveform::addcontent(double y, double x)
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

