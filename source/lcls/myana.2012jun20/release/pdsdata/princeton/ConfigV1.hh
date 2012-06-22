#ifndef PRINCETON_CONFIG_V1_HH
#define PRINCETON_CONFIG_V1_HH

#include <stdint.h>

#pragma pack(4)

namespace Pds 
{

namespace Princeton 
{

class ConfigV2; // forward declaration
class ConfigV3; // forward declaration

class ConfigV1 
{
public:
  enum { Version = 1 };

  ConfigV1()  {}  
  ConfigV1(
   uint32_t         uWidth, 
   uint32_t         uHeight, 
   uint32_t         uOrgX, 
   uint32_t         uOrgY, 
   uint32_t         uBinX, 
   uint32_t         uBinY,
   float            f32ExposureTime, 
   float            f32CoolingTemp, 
   uint32_t         u32ReadoutSpeedIndex,
   uint16_t         u16ReadoutEventCode, 
   uint16_t         u16DelayMode          = -1  // default value is used by the config program
   );

  uint32_t          width ()            const         { return _uWidth; }
  uint32_t          height()            const         { return _uHeight; }
  uint32_t          orgX  ()            const         { return _uOrgX; }
  uint32_t          orgY  ()            const         { return _uOrgY; }    
  uint32_t          binX  ()            const         { return _uBinX; }
  uint32_t          binY  ()            const         { return _uBinY; }    
  float             exposureTime()      const         { return _f32ExposureTime; }
  float             coolingTemp ()      const         { return _f32CoolingTemp; }
  uint32_t          readoutSpeedIndex() const         { return _u32ReadoutSpeedIndex; }
  
  uint16_t          readoutEventCode()  const         { return _u16ReadoutEventCode; }
  uint16_t          delayMode()         const         { return _u16DelayMode; }

  uint32_t          setWidth    (uint32_t uWidth)     { return _uWidth = uWidth; }
  uint32_t          setHeight   (uint32_t uHeight)    { return _uHeight = uHeight; }
  uint16_t          setDelayMode(uint16_t uDelayMode) { return _u16DelayMode = uDelayMode; }
  
  int               size      ()        const         { return sizeof(*this); }
  int               frameSize ()        const; // calculate the frame size based on the current ROI and binning settings
  
private:
  uint32_t          _uWidth, _uHeight;
  uint32_t          _uOrgX,  _uOrgY;
  uint32_t          _uBinX,  _uBinY;
  float             _f32ExposureTime;
  float             _f32CoolingTemp;
  uint32_t          _u32ReadoutSpeedIndex;
  uint16_t          _u16ReadoutEventCode;
  uint16_t          _u16DelayMode;
  
  friend class ConfigV2;
  friend class ConfigV3;
};

} // namespace Princeton

} // namespace Pds 

#pragma pack()

#endif
