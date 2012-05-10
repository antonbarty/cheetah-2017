#include "pdsdata/cspad2x2/ConfigV1.hh"

using namespace Pds::CsPad2x2;

ConfigV1::ConfigV1(
		   uint32_t inactiveRunMode,
		   uint32_t activeRunMode,
		   uint32_t testDataIndex,
		   uint32_t payloadPerQuad,
		   uint32_t badAsicMask,
		   uint32_t AsicMask,
		   uint32_t roiMask) :
  _concentratorVersion(0),
  _inactiveRunMode(inactiveRunMode),
  _activeRunMode(activeRunMode),
  _testDataIndex(testDataIndex),
  _payloadPerQuad(payloadPerQuad),
  _badAsicMask(badAsicMask),
  _AsicMask(AsicMask),
  _roiMask (roiMask)
{
}

ProtectionSystemThreshold* ConfigV1::protectionThreshold ()
{
  return &_protectionThreshold;
}

const ProtectionSystemThreshold* ConfigV1::protectionThreshold () const
{
  return &_protectionThreshold;
}

uint32_t* ConfigV1::concentratorVersionAddr()
{
  return &_concentratorVersion;
}

ConfigV1QuadReg* ConfigV1::quad()
{
  return &_quad;
}

const ConfigV1QuadReg* ConfigV1::quad() const
{
  return &_quad;
}

unsigned ConfigV1::roiMask      (int iq) const
{
  return _roiMask&0x3;
}

unsigned ConfigV1::roiMask      () const
{
  return _roiMask&0x3;
}

unsigned ConfigV1::numAsicsRead () const
{
  return  4;
}

unsigned ConfigV1::numAsicsStored(int iq) const
{
  unsigned m = roiMask(0);
  unsigned c;
  for(c=0; m; c++)
    m &= m-1;
  return c<<1;
}
unsigned ConfigV1::numAsicsStored() const
{
  return ConfigV1::numAsicsStored(0);
}
