#ifndef PRINCETON_FRAME_V1_HH
#define PRINCETON_FRAME_V1_HH

#include <stdio.h>
#include <stdint.h>
#include <stdexcept>

#pragma pack(4)

namespace Pds 
{
  
namespace Princeton 
{

class FrameV1 
{
public:
  static const int Version = 1;

  FrameV1( uint32_t  iShotIdStart, float fReadoutTime );    
  
  uint32_t          shotIdStart () const { return _iShotIdStart; }
  float             readoutTime () const { return _fReadoutTime; }  

  const uint16_t*   data        ()        const;
  
private:  
  uint32_t  _iShotIdStart;
  float     _fReadoutTime;
};


} // namespace Princeton

} // namespace Pds 

#pragma pack()

#endif
