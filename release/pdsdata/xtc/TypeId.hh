#ifndef Pds_TypeId_hh
#define Pds_TypeId_hh

#include <stdint.h>

namespace Pds {

  class TypeId {
  public:
    /*
     * Notice: New enum values should be appended to the end of the enum list, since
     *   the old values have already been recorded in the existing xtc files. 
     */
    enum Type {
      Any, 
      Id_Xtc,          // generic hierarchical container
      Id_Frame,        // raw image
      Id_AcqWaveform,
      Id_AcqConfig,
      Id_TwoDGaussian, // 2-D Gaussian + covariances
      Id_Opal1kConfig,
      Id_FrameFexConfig,
      Id_EvrConfig,
      Id_TM6740Config,
      Id_ControlConfig,
      Id_pnCCDframe,
      Id_pnCCDconfig,
      Id_Epics,        // Epics Data Type
      Id_FEEGasDetEnergy,
      Id_EBeam,
      Id_PhaseCavity,
      Id_PrincetonFrame,
      Id_PrincetonConfig,
      Id_EvrData,
      Id_FrameFccdConfig,
      Id_FccdConfig,
      Id_IpimbData,
      Id_IpimbConfig,
      Id_EncoderData,
      Id_EncoderConfig,
      Id_EvrIOConfig,
      Id_PrincetonInfo,
      Id_CspadElement,
      Id_CspadConfig,
      Id_IpmFexConfig,  // LUSI Diagnostics
      Id_IpmFex,
      Id_DiodeFexConfig,
      Id_DiodeFex,
      Id_PimImageConfig,
      Id_SharedIpimb,
      Id_AcqTdcConfig,
      Id_AcqTdcData,
      Id_Index,
      Id_XampsConfig,
      Id_XampsElement,
      NumberOf};

    TypeId() {}
    TypeId(const TypeId& v);
    TypeId(Type type, uint32_t version);

    Type     id()      const;
    uint32_t version() const;
    uint32_t value()   const;

    static const char* name(Type type);
    
  private:
    uint32_t _value;
  };

}

#endif
