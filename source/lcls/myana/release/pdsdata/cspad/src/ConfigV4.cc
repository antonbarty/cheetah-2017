#include "pdsdata/cspad/ConfigV4.hh"

using namespace Pds::CsPad;

ConfigV4::ConfigV4(
		   uint32_t runDelay,
		   uint32_t eventCode,
		   uint32_t inactiveRunMode,
		   uint32_t activeRunMode,
		   uint32_t testDataIndex,
		   uint32_t payloadPerQuad,
		   uint32_t badAsicMask0,
		   uint32_t badAsicMask1,
		   uint32_t AsicMask,
		   uint32_t quadMask,
		   uint32_t roiMask) :
  _concentratorVersion(0),
  _runDelay(runDelay),
  _eventCode(eventCode),
  _inactiveRunMode(inactiveRunMode),
  _activeRunMode(activeRunMode),
  _testDataIndex(testDataIndex),
  _payloadPerQuad(payloadPerQuad),
  _badAsicMask0(badAsicMask0),
  _badAsicMask1(badAsicMask1),
  _AsicMask(AsicMask),
  _quadMask(quadMask),
  _roiMask (roiMask)
{
}

unsigned ConfigV4::roiMask      (int iq) const
{
  return (_roiMask>>(8*iq))&0xff; 
}

unsigned ConfigV4::numAsicsRead () const
{
  return (_AsicMask&0xf==1) ? 4 : 16;
}

unsigned ConfigV4::numAsicsStored(int iq) const
{
  unsigned m = roiMask(iq);
  unsigned c;
  for(c=0; m; c++)
    m &= m-1;
  return c<<1;
}
