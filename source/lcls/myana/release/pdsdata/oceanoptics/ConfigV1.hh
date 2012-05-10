#ifndef OCEANOPTICS_CONFIG_V1_HH
#define OCEANOPTICS_CONFIG_V1_HH

#include <stdint.h>

#pragma pack(4)

namespace Pds 
{

namespace OceanOptics 
{

class ConfigV1 
{
public:
  enum { Version = 1 };

  ConfigV1()  {}  
  ConfigV1(float f32ExposureTime);
  ConfigV1(float f32ExposureTime, double lfWaveLenCalibCoeff[4], double fStrayLightConstant, double lfNonlinCorrectCoeff[8]);

  int               size              ()            const { return sizeof(*this); }    
  float             exposureTime      ()            const { return _f32ExposureTime; }
  double            waveLenCalib      (int iOrder)  const { return _lfWaveLenCalibCoeff[iOrder]; }
  double            strayLightConstant()            const { return _fStrayLightConstant; }
  double            nonlinCorrect     (int iOrder)  const { return _lfNonlinCorrectCoeff[iOrder]; }  
  
private:
  float             _f32ExposureTime;
  double            _lfWaveLenCalibCoeff[4];  // Wavelength Calibration Coefficient. 0th - 3rd order
  double            _lfNonlinCorrectCoeff[8]; // Non-linearity correction coefficient. 0th - 7th order
  double            _fStrayLightConstant;  
};

} // namespace OceanOptics

} // namespace Pds 

#pragma pack()

#endif
