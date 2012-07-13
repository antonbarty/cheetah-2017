#include "pdsdata/princeton/ConfigV3.hh"
#include "pdsdata/princeton/FrameV1.hh"

#include <string.h>

using namespace Pds;
using namespace Princeton;

ConfigV3::ConfigV3(
 uint32_t         uWidth, 
 uint32_t         uHeight, 
 uint32_t         uOrgX, 
 uint32_t         uOrgY, 
 uint32_t         uBinX, 
 uint32_t         uBinY,
 float            f32ExposureTime, 
 float            f32CoolingTemp, 
 uint8_t          u8GainIndex,
 uint8_t          u8ReadoutSpeedIndex,
 uint16_t         u16ExposureEventCode, 
 uint32_t         u32NumDelayShots) :
 _uWidth  (uWidth), 
 _uHeight (uHeight), 
 _uOrgX   (uOrgX), 
 _uOrgY   (uOrgY), 
 _uBinX   (uBinX), 
 _uBinY   (uBinY),
 _f32ExposureTime       (f32ExposureTime),
 _f32CoolingTemp        (f32CoolingTemp), 
 _u8GainIndex           (u8GainIndex), 
 _u8ReadoutSpeedIndex   (u8ReadoutSpeedIndex), 
 _u16ExposureEventCode   (u16ExposureEventCode),
 _u32NumDelayShots          (u32NumDelayShots)
 {} 
 
ConfigV3::ConfigV3(const ConfigV1& configV1) :
 _uWidth  (configV1._uWidth), 
 _uHeight (configV1._uHeight), 
 _uOrgX   (configV1._uOrgX), 
 _uOrgY   (configV1._uOrgY), 
 _uBinX   (configV1._uBinX), 
 _uBinY   (configV1._uBinY),
 _f32ExposureTime       (configV1._f32ExposureTime),
 _f32CoolingTemp        (configV1._f32CoolingTemp), 
 _u8GainIndex           (0), 
 _u8ReadoutSpeedIndex   (configV1._u32ReadoutSpeedIndex), 
 _u16ExposureEventCode  (configV1._u16ReadoutEventCode),
 _u32NumDelayShots      (configV1._u16DelayMode)
{  
}

ConfigV3::ConfigV3(const ConfigV2& configV2) :
 _uWidth  (configV2._uWidth), 
 _uHeight (configV2._uHeight), 
 _uOrgX   (configV2._uOrgX), 
 _uOrgY   (configV2._uOrgY), 
 _uBinX   (configV2._uBinX), 
 _uBinY   (configV2._uBinY),
 _f32ExposureTime       (configV2._f32ExposureTime),
 _f32CoolingTemp        (configV2._f32CoolingTemp), 
 _u8GainIndex           (configV2._u16GainIndex), 
 _u8ReadoutSpeedIndex   (configV2._u16ReadoutSpeedIndex), 
 _u16ExposureEventCode  (configV2._u16ReadoutEventCode),
 _u32NumDelayShots      (configV2._u16DelayMode)
{  
}

int ConfigV3::frameSize() const
{
  return sizeof(FrameV1) + 
    (int) ((_uWidth + _uBinX-1)/ _uBinX ) * 
    (int) ((_uHeight+ _uBinY-1)/ _uBinY ) * 2; // 2 -> 16 bit color depth
  //return sizeof(FrameV1) + 4*1024*1024*2; // 2 -> 16 bit color depth // !! debug
}
