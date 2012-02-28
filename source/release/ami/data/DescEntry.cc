#include <string.h>

#include "ami/data/DescEntry.hh"

using namespace Ami;
using Pds::DetInfo;

DescEntry::DescEntry(const char* name,
		     const char* xtitle, 
		     const char* ytitle, 
		     Type type, 
		     unsigned short size,
		     bool isnormalized,
		     bool doaggregate,
                     unsigned options) :
  Desc(name),
  _info(0,DetInfo::NoDetector,0,DetInfo::NoDevice,0),
  _channel(-1),
  _group(-1),
  _options(options<<User),
  _type(type),
  _size(size)
{ 
  normalize(isnormalized);
  aggregate(doaggregate);

  strncpy(_xtitle, xtitle, TitleSize);
  _xtitle[TitleSize-1] = 0;
  strncpy(_ytitle, ytitle, TitleSize);
  _ytitle[TitleSize-1] = 0;
  _zunits[0] = 0;
}

DescEntry::DescEntry(const Pds::DetInfo& info,
		     unsigned channel,
		     const char* name,
		     const char* xtitle, 
		     const char* ytitle, 
		     Type type, 
		     unsigned short size,
		     bool isnormalized,
		     bool doaggregate,
                     unsigned options) :
  Desc(name),
  _info   (info),
  _channel(channel),
  _group(-1),
  _options(options<<User),
  _type(type),
  _size(size)
{ 
  normalize(isnormalized);
  aggregate(doaggregate);

  strncpy(_xtitle, xtitle, TitleSize);
  _xtitle[TitleSize-1] = 0;
  strncpy(_ytitle, ytitle, TitleSize);
  _ytitle[TitleSize-1] = 0;
  _zunits[0] = 0;
}

DescEntry::DescEntry(const Pds::DetInfo& info,
		     unsigned channel,
		     const char* name,
		     const char* xtitle, 
		     const char* ytitle, 
                     const char* zunits,
		     Type type, 
		     unsigned short size,
		     bool isnormalized,
		     bool doaggregate,
                     bool hasPedCalib,
                     bool hasGainCalib, 
                     bool hasRmsCalib,
                     unsigned options) :
  Desc(name),
  _info   (info),
  _channel(channel),
  _group(-1),
  _options(options<<User),
  _type(type),
  _size(size)
{ 
  normalize(isnormalized);
  aggregate(doaggregate);

  if (hasPedCalib ) _options |= 1<<CalibMom0;
  if (hasGainCalib) _options |= 1<<CalibMom1;
  if (hasRmsCalib ) _options |= 1<<CalibMom2;

  strncpy(_xtitle, xtitle, TitleSize);
  _xtitle[TitleSize-1] = 0;
  strncpy(_ytitle, ytitle, TitleSize);
  _ytitle[TitleSize-1] = 0;
  strncpy(_zunits, zunits, TitleSize);
  _zunits[TitleSize-1] = 0;
}



DescEntry::Type DescEntry::type() const {return Type(_type);}
unsigned short DescEntry::size() const {return _size;}
const char* DescEntry::xtitle() const {return _xtitle;}
const char* DescEntry::ytitle() const {return _ytitle;}
const char* DescEntry::zunits() const {return _zunits;}

const Pds::DetInfo& DescEntry::info() const { return _info; }
unsigned            DescEntry::channel() const { return _channel; }

bool DescEntry::isnormalized() const {return _options&(1<<Normalized);}
bool DescEntry::aggregate   () const {return _options&(1<<Aggregate);}
bool DescEntry::isweighted_type() const {return (_type==Scan);}
bool DescEntry::hasPedCalib    () const {return _options&(1<<CalibMom0);}
bool DescEntry::hasGainCalib   () const {return _options&(1<<CalibMom1);}
bool DescEntry::hasRmsCalib    () const {return _options&(1<<CalibMom2);}

void DescEntry::normalize(bool v) {
  if (v) _options |=  (1<<Normalized);
  else   _options &= ~(1<<Normalized);
}

void DescEntry::aggregate(bool v) {
  if (v) _options |=  (1<<Aggregate);
  else   _options &= ~(1<<Aggregate);
}

void DescEntry::pedcalib(bool v) {
  if (v) _options |=  (1<<CalibMom0);
  else   _options &= ~(1<<CalibMom0);
}

void DescEntry::gaincalib(bool v) {
  if (v) _options |=  (1<<CalibMom1);
  else   _options &= ~(1<<CalibMom1);
}

void DescEntry::rmscalib(bool v) {
  if (v) _options |=  (1<<CalibMom2);
  else   _options &= ~(1<<CalibMom2);
}

void DescEntry::options(unsigned o) {
  _options &= (1<<User)-1;
  _options |= o<<User;
}

void DescEntry::xwarnings(float warn, float err) 
{
  _options |= (1<<XWarnings);
  _xwarn = warn;
  _xerr = err;
}

void DescEntry::ywarnings(float warn, float err) 
{
  _options |= (1<<YWarnings);
  _ywarn = warn;
  _yerr = err;
}

bool DescEntry::xhaswarnings() const {return _options&(1<<XWarnings);}
bool DescEntry::yhaswarnings() const {return _options&(1<<YWarnings);}

float DescEntry::xwarn() const {return _xwarn;}
float DescEntry::xerr() const {return _xerr;}

float DescEntry::ywarn() const {return _ywarn;}
float DescEntry::yerr() const {return _yerr;}
