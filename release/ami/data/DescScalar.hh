#ifndef Pds_ENTRYDESCScalar_HH
#define Pds_ENTRYDESCScalar_HH

#include "ami/data/DescEntryW.hh"

namespace Ami {

  class DescScalar : public DescEntryW {
  public:
    DescScalar(const char* name, 
	       const char* ytitle,
	       const char* weight="");

    DescScalar(const Pds::DetInfo& info,
	       unsigned channel,
	       const char* name, 
	       const char* ytitle);
  };
};

#endif
