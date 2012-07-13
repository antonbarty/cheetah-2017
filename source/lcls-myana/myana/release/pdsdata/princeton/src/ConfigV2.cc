#include "pdsdata/princeton/ConfigV2.hh"
#include "pdsdata/princeton/FrameV1.hh"

#include <string.h>

using namespace Pds;
using namespace Princeton;

ConfigV2::ConfigV2(
 uint32_t         uWidth, 
 uint32_t         uHeight, 
 uint32_t         uOrgX, 
 uint32_t         uOrgY, 
 uint32_t         uBinX, 
 uint32_t         uBinY,
 float            f32ExposureTime, 
 float            f32CoolingTemp, 
 uint16_t         u16GainIndex,
 uint16_t         u16ReadoutSpeedIndex,
 uint16_t         u16ReadoutEventCode, 
 uint16_t         u16DelayMode) :
 _uWidth  (uWidth), 
 _uHeight (uHeight), 
 _uOrgX   (uOrgX), 
 _uOrgY   (uOrgY), 
 _uBinX   (uBinX), 
 _uBinY   (uBinY),
 _f32ExposureTime       (f32ExposureTime),
 _f32CoolingTemp        (f32CoolingTemp), 
 _u16GainIndex          (u16GainIndex), 
 _u16ReadoutSpeedIndex  (u16ReadoutSpeedIndex), 
 _u16ReadoutEventCode   (u16ReadoutEventCode),
 _u16DelayMode          (u16DelayMode)
 {}
 
ConfigV2::ConfigV2(const ConfigV1& configV1) :
 _uWidth  (configV1._uWidth), 
 _uHeight (configV1._uHeight), 
 _uOrgX   (configV1._uOrgX), 
 _uOrgY   (configV1._uOrgY), 
 _uBinX   (configV1._uBinX), 
 _uBinY   (configV1._uBinY),
 _f32ExposureTime       (configV1._f32ExposureTime),
 _f32CoolingTemp        (configV1._f32CoolingTemp), 
 _u16GainIndex          (0), 
 _u16ReadoutSpeedIndex  (configV1._u32ReadoutSpeedIndex), 
 _u16ReadoutEventCode   (configV1._u16ReadoutEventCode),
 _u16DelayMode          (configV1._u16DelayMode)
{  
}

int ConfigV2::frameSize() const
{
  return sizeof(FrameV1) + 
    (int) ((_uWidth + _uBinX-1)/ _uBinX ) * 
    (int) ((_uHeight+ _uBinY-1)/ _uBinY ) * 2; // 2 -> 16 bit color depth
  //return sizeof(FrameV1) + 4*1024*1024*2; // 2 -> 16 bit color depth // !! debug
}
