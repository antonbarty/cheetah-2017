#include "pdsdata/pnCCD/ConfigV1.hh"

#include <string.h>

using namespace Pds;
using namespace PNCCD;

ConfigV1::ConfigV1() {}

ConfigV1::ConfigV1(uint32_t numLinks, uint32_t payloadSizePerLink) : 
  _numLinks(numLinks), _payloadSizePerLink(payloadSizePerLink) {}

uint32_t ConfigV1::numLinks()           const {return _numLinks;}
uint32_t ConfigV1::payloadSizePerLink() const {return _payloadSizePerLink;} // bytes
unsigned ConfigV1::size() const { return sizeof(*this); }
