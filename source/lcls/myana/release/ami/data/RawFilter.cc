#include "RawFilter.hh"

using namespace Ami;

RawFilter::RawFilter() : AbsFilter(AbsFilter::Raw) {}
RawFilter::~RawFilter() {}

bool RawFilter::accept() const { return true; }
void* RawFilter::_serialize(void* p) const { return p; }

AbsFilter* RawFilter::clone() const { return new RawFilter; }
