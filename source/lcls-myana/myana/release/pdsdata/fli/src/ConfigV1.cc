#include "pdsdata/fli/ConfigV1.hh"
#include "pdsdata/fli/FrameV1.hh"

#include <string.h>

using namespace Pds;
using namespace Fli;

ConfigV1::ConfigV1(
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
 
int ConfigV1::frameSize() const
{
  // Note: This formula is different from Princeton image
  return sizeof(FrameV1) + 
    (int) (_uWidth / _uBinX ) * 
    (int) (_uHeight/ _uBinY ) * 2; // 2 -> 16 bit color depth  
  //return sizeof(FrameV1) + 4*1024*1024*2; // 2 -> 16 bit color depth // !! debug
}
