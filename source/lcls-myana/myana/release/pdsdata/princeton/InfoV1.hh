#ifndef PRINCETON_INFO_V1_HH
#define PRINCETON_INFO_V1_HH

#include <stdio.h>
#include <stdint.h>
#include <stdexcept>

#pragma pack(4)

namespace Pds 
{
  
namespace Princeton 
{

class InfoV1 
{
public:
  static const int Version = 1;

  InfoV1( float fTemperature );
  
  float temperature() const { return _fTemperature; }  
  
private:  
  float _fTemperature;
};


} // namespace Princeton

} // namespace Pds 

#pragma pack()

#endif
