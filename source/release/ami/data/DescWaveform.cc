#include "ami/data/DescWaveform.hh"

using namespace Ami;

DescWaveform::DescWaveform(const Pds::DetInfo& info,
			   unsigned channel,
			   const char* name, 
			   const char* xtitle, 
			   const char* ytitle, 
			   unsigned nbins, 
			   float xlow, 
			   float xup) :
  DescEntry(info, channel, name, xtitle, ytitle, Waveform, sizeof(DescWaveform), true),
  _nbins(nbins ? nbins : 1),
  _xlow(xlow),
  _xup(xup)
{}

void DescWaveform::params(unsigned nbins, float xlow, float xup)
{
  _nbins = nbins ? nbins : 1;
  _xlow = xlow;
  _xup = xup;
}
