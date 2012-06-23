#ifndef Evr_PulseConfigV3_hh
#define Evr_PulseConfigV3_hh

#include <stdint.h>

#pragma pack(4)

namespace Pds
{
namespace EvrData
{

class PulseConfigV3
{
public:
  PulseConfigV3(
    uint16_t  u16PulseId,  
    uint16_t  u16Polarity,      // 0 -> positive polarity , 1 -> negative polarity
    uint32_t  u32Prescale = 1,  // Clock divider
    uint32_t  u32Delay    = 0,  // Delay in 119MHz clks
    uint32_t  u32Width    = 0   // Width in 119MHz clks
    );
    
  PulseConfigV3() {} // For array initialization
    
  uint16_t  pulseId () const { return _u16PulseId; }
  uint16_t  polarity() const { return _u16Polarity; }
  uint32_t  prescale() const { return _u32Prescale; }
  uint32_t  delay   () const { return _u32Delay; }
  uint32_t  width   () const { return _u32Width; }
  
  void      setPulseId(uint16_t u16PulseId) { _u16PulseId = u16PulseId; }
  
private:
  uint16_t  _u16PulseId;
  uint16_t  _u16Polarity;
  uint32_t  _u32Prescale;
  uint32_t  _u32Delay;
  uint32_t  _u32Width;  
};

} // namespace EvrData
} // namespace Pds

#pragma pack()

#endif
