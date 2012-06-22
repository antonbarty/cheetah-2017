#include "ami/data/DescRef.hh"

using namespace Ami;

DescRef::DescRef(const char* name, 
		 const char* ytitle) :
  DescEntry(name, "", ytitle, Ref, sizeof(DescRef))
{}

DescRef::DescRef(const Pds::DetInfo& info,
		 unsigned channel,
		 const char* name, 
		 const char* ytitle) :
  DescEntry(info, channel, name, "", ytitle, Ref, sizeof(DescRef))
{}
