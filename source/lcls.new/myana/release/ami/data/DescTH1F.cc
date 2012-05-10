#include "ami/data/DescTH1F.hh"

using namespace Ami;

DescTH1F::DescTH1F(const char* name, 
		   const char* xtitle, 
		   const char* ytitle, 
		   unsigned nbins, 
		   float xlow, 
		   float xup,
		   bool normalize) :
  DescEntry(name, xtitle, ytitle, TH1F, sizeof(DescTH1F), normalize),
  _nbins(nbins ? nbins : 1),
  _xlow(xlow),
  _xup(xup)
{}

DescTH1F::DescTH1F(const Pds::DetInfo& info,
		   unsigned channel,
		   const char* name, 
		   const char* xtitle, 
		   const char* ytitle, 
		   unsigned nbins, 
		   float xlow, 
		   float xup,
		   bool normalize) :
  DescEntry(info, channel, name, xtitle, ytitle, TH1F, sizeof(DescTH1F), normalize),
  _nbins(nbins ? nbins : 1),
  _xlow(xlow),
  _xup(xup)
{}

void DescTH1F::params(unsigned nbins, float xlow, float xup)
{
  _nbins = nbins ? nbins : 1;
  _xlow = xlow;
  _xup = xup;
}
