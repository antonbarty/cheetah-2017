#ifndef FLI_FRAME_V1_HH
#define FLI_FRAME_V1_HH

#include <stdio.h>
#include <stdint.h>
#include <stdexcept>

#pragma pack(4)

namespace Pds 
{
  
namespace Fli 
{

class FrameV1 
{
public:
  static const int Version = 1;

  FrameV1( uint32_t  iShotIdStart, float fReadoutTime );    
  
  uint32_t          shotIdStart () const { return _iShotIdStart; }
  float             readoutTime () const { return _fReadoutTime; }  
  float             temperature () const { return _fTemperature; }  

  const uint16_t*   data        ()        const;
  
  void              setTemperature(float fTemperature) {_fTemperature = fTemperature;}
  
private:  
  uint32_t  _iShotIdStart;
  float     _fReadoutTime;
  float     _fTemperature;
};


} // namespace Fli

} // namespace Pds 

#pragma pack()

#endif
