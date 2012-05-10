#include "ami/data/DescProf.hh"

using namespace Ami;

DescProf::DescProf(const char* name, 
		   const char* xtitle, 
		   const char* ytitle, 
		   unsigned nbins, 
		   float xlow, 
		   float xup, 
		   const char* names,
		   const char* weight) :
  DescEntryW(name, xtitle, ytitle, weight, Prof, sizeof(DescProf), false),
  _nbins(nbins ? nbins : 1),
  _xlow(xlow),
  _xup(xup)
{
  if (names) {
    char* dst = _names;
    const char* last = dst+NamesSize-1;
    while (*names && dst < last) {
      *dst = *names != ':' ? *names : 0;
      dst++;
      names++;
    }
    *dst++ = 0;
    *dst++ = 0;
  } else {
    _names[0] = 0;
  }
}

DescProf::DescProf(const Pds::DetInfo& info,
		   unsigned channel,
		   const char* name, 
		   const char* xtitle, 
		   const char* ytitle, 
		   unsigned nbins, 
		   float xlow, 
		   float xup, 
		   const char* names) :
  DescEntryW(info, channel, name, xtitle, ytitle, "", Prof, sizeof(DescProf), false),
  _nbins(nbins ? nbins : 1),
  _xlow(xlow),
  _xup(xup)
{
  if (names) {
    char* dst = _names;
    const char* last = dst+NamesSize-1;
    while (*names && dst < last) {
      *dst = *names != ':' ? *names : 0;
      dst++;
      names++;
    }
    *dst++ = 0;
    *dst++ = 0;
  } else {
    _names[0] = 0;
  }
}

void DescProf::params(unsigned nbins, 
		      float xlow, 
		      float xup, 
		      const char* names)
{
  _nbins = nbins ? nbins : 1;
  _xlow = xlow;
  _xup = xup;
  if (names) {
    char* dst = _names;
    const char* last = dst+NamesSize-1;
    while (*names && dst < last) {
      *dst = *names != ':' ? *names : 0;
      dst++;
      names++;
    }
    *dst++ = 0;
    *dst++ = 0;
  } else {
    _names[0] = 0;
  }
}
