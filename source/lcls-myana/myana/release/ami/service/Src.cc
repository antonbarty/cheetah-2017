#include "ami/service/Src.hh"

using namespace Ami;

Src::Src(unsigned id) : Pds::Src(Pds::Level::Observer)
{
  _phy=id;
}

Src::~Src() {}

unsigned Src::id() const { return _phy; }
