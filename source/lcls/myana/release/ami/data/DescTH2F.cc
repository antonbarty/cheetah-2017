#include "ami/data/DescTH2F.hh"

using namespace Ami;

DescTH2F::DescTH2F(const char* name, 
		   const char* xtitle, 
		   const char* ytitle, 
		   unsigned nbinsx, 
		   float xlow, 
		   float xup,
		   unsigned nbinsy, 
		   float ylow, 
		   float yup,
		   bool isnormalized) :
  DescEntry(name, xtitle, ytitle, TH2F, sizeof(DescTH2F),
	    isnormalized),
  _nbinsx(nbinsx ? nbinsx : 1),
  _nbinsy(nbinsy ? nbinsy : 1),
  _xlow(xlow),
  _xup(xup),
  _ylow(ylow),
  _yup(yup)
{}

DescTH2F::DescTH2F(const Pds::DetInfo& info,
		   unsigned channel,
		   const char* name, 
		   const char* xtitle, 
		   const char* ytitle, 
		   unsigned nbinsx, 
		   float xlow, 
		   float xup,
		   unsigned nbinsy, 
		   float ylow, 
		   float yup,
		   bool isnormalized) :
  DescEntry(info, channel, name, xtitle, ytitle, TH2F, sizeof(DescTH2F),
	    isnormalized),
  _nbinsx(nbinsx ? nbinsx : 1),
  _nbinsy(nbinsy ? nbinsy : 1),
  _xlow(xlow),
  _xup(xup),
  _ylow(ylow),
  _yup(yup)
{}

void DescTH2F::params(unsigned nbinsx, float xlow, float xup,
			     unsigned nbinsy, float ylow, float yup)
{
  _nbinsx = nbinsx ? nbinsx : 1;
  _xlow = xlow;
  _xup = xup;
  _nbinsy = nbinsy ? nbinsy : 1;
  _ylow = ylow;
  _yup = yup;
}
