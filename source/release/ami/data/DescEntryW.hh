#ifndef Pds_DescEntryW_hh
#define Pds_DescEntryW_hh

#include "ami/data/DescEntry.hh"

#include <string.h>

namespace Ami {

  class DescEntryW : public DescEntry {
  public:
    DescEntryW(const char* name, const char* xtitle, const char* ytitle, const char* weight,
	       Type type, unsigned short size, bool isnormalized=true, bool aggregate=true);

    DescEntryW(const Pds::DetInfo& info, unsigned channel,
	       const char* name, const char* xtitle, const char* ytitle, const char* weight,
	       Type type, unsigned short size, bool isnormalized=true, bool aggregate=true);

    bool        weighted() const;
    const char* weight  () const;

  private:
    enum { WeightSize=64 };
    char _weight[WeightSize];
  };

  inline     DescEntryW::DescEntryW(const char* name, const char* xtitle, const char* ytitle, const char* weight,
				    Type type, unsigned short size, bool isnormalized, bool aggregate) :
    DescEntry(name, xtitle, ytitle, type, size, isnormalized, aggregate) 
  { strncpy(_weight,weight,WeightSize); }

  inline DescEntryW::DescEntryW(const Pds::DetInfo& info, unsigned channel,
				const char* name, const char* xtitle, const char* ytitle, const char* weight,
				Type type, unsigned short size, bool isnormalized, bool aggregate) :
    DescEntry(info, channel, name, xtitle, ytitle, type, size, isnormalized, aggregate) 
  { strncpy(_weight,weight,WeightSize); }
  
  inline bool        DescEntryW::weighted() const { return _weight[0]!=0; }

  inline const char* DescEntryW::weight  () const { return _weight; }
};

#endif
