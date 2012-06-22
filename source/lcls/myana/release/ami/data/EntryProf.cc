#include "ami/data/EntryProf.hh"

static const unsigned DefaultNbins = 1;
static const float DefaultLo = 0;
static const float DefaultUp = 1;
static const char* DefaultNames = 0;

using namespace Ami;

#define SIZE(nb) (3*nb+InfoSize)

EntryProf::~EntryProf() {}

EntryProf::EntryProf(const Pds::DetInfo& info, unsigned channel, 
		     const char* name, 
		     const char* xtitle, 
		     const char* ytitle) :
  _desc(info, channel, name, xtitle, ytitle, DefaultNbins, DefaultLo, DefaultUp, DefaultNames)
{
  build(DefaultNbins);
}

EntryProf::EntryProf(const DescProf& desc) :
  _desc(desc)
{
  build(desc.nbins());
}

void EntryProf::params(unsigned nbins, 
			  float xlow, 
			  float xup, 
			  const char* names)
{
  _desc.params(nbins, xlow, xup, names);
  build(nbins);
}

void EntryProf::params(const DescProf& desc)
{
  _desc = desc;
  build(_desc.nbins());
}

void EntryProf::build(unsigned nbins)
{
  _ysum = static_cast<double*>(allocate(SIZE(nbins)*sizeof(double)));
  _y2sum = _ysum+nbins;
  _nentries = _y2sum+nbins;
}

const DescProf& EntryProf::desc() const {return _desc;}
DescProf& EntryProf::desc() {return _desc;}

void EntryProf::setto(const EntryProf& entry) 
{
  double* dst = _ysum;
  const double* end = dst + SIZE(_desc.nbins());
  const double* src = entry._ysum;
  do {
    *dst++ = *src++;
  } while (dst < end);
  valid(entry.time());
}

void EntryProf::diff(const EntryProf& curr, 
		     const EntryProf& prev) 
{
  double* dst = _ysum;
  const double* end = dst + SIZE(_desc.nbins());
  const double* srccurr = curr._ysum;
  const double* srcprev = prev._ysum;
  do {
    *dst++ = *srccurr++ - *srcprev++;
  } while (dst < end);
  valid(curr.time());
}

void EntryProf::sum(const EntryProf& curr, 
		    const EntryProf& prev) 
{
  double* dst = _ysum;
  const double* end = dst + SIZE(_desc.nbins());
  const double* srccurr = curr._ysum;
  const double* srcprev = prev._ysum;
  do {
    *dst++ = *srccurr++ + *srcprev++;
  } while (dst < end);
  valid(curr.time());
}
