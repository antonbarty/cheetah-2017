#include "ami/data/EntryRef.hh"

using namespace Ami;

EntryRef::~EntryRef() {}

EntryRef::EntryRef(const Pds::DetInfo& info, unsigned channel,
		   const char* name, 
		   const char* ytitle) :
  _desc(info, channel, name, ytitle),
  _y   (allocate(sizeof(void*)))
{
}

EntryRef::EntryRef(const DescRef& desc) :
  _desc(desc),
  _y   (allocate(sizeof(void*)))
{
}
