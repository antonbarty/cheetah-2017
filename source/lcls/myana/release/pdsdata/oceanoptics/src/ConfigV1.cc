#include "pdsdata/oceanoptics/ConfigV1.hh"

#include <memory.h>

namespace Pds 
{

namespace OceanOptics 
{

ConfigV1::ConfigV1(float f32ExposureTime, double lfWaveLenCalibCoeff[4], double fStrayLightConstant, double lfNonlinCorrectCoeff[8]) :
  _f32ExposureTime    (f32ExposureTime),
  _fStrayLightConstant(fStrayLightConstant)
 {
   memcpy(_lfWaveLenCalibCoeff, lfWaveLenCalibCoeff, sizeof(_lfWaveLenCalibCoeff));
   memcpy(_lfNonlinCorrectCoeff, lfNonlinCorrectCoeff, sizeof(_lfNonlinCorrectCoeff));
 }
 
ConfigV1::ConfigV1(float f32ExposureTime) :
  _f32ExposureTime    (f32ExposureTime),
  _fStrayLightConstant(0)
 {
   memset(_lfWaveLenCalibCoeff , 0, sizeof(_lfWaveLenCalibCoeff));
   memset(_lfNonlinCorrectCoeff, 0, sizeof(_lfNonlinCorrectCoeff));
 }
 

} // namespace OceanOptics

} // namespace Pds 
