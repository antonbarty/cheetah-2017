#include "ami/data/DescScalar.hh"

using namespace Ami;

DescScalar::DescScalar(const char* name, 
		       const char* ytitle,
		       const char* weight) :
  DescEntryW(name, "", ytitle, weight, Scalar, sizeof(DescScalar), true)
{}

DescScalar::DescScalar(const Pds::DetInfo& info,
		       unsigned channel,
		       const char* name, 
		       const char* ytitle) :
  DescEntryW(info, channel, name, "", ytitle, "", Scalar, sizeof(DescScalar), true)
{}
