//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class DumpBld...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana_examples/DumpBld.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
#include "psddl_psana/bld.ddl.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

using namespace psana_examples;
PSANA_MODULE_FACTORY(DumpBld)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana_examples {

//----------------
// Constructors --
//----------------
DumpBld::DumpBld (const std::string& name)
  : Module(name)
{
  m_ebeamSrc = configStr("eBeamSource", "BldInfo(EBeam)");
  m_cavSrc = configStr("phaseCavSource", "BldInfo(PhaseCavity)");
  m_feeSrc = configStr("feeSource", "BldInfo(FEEGasDetEnergy)");
  m_ipimbSrc = configStr("ipimbSource", "BldInfo(NH2-SB1-IPM-01)");
  m_pimSrc = configStr("pimSource", "BldInfo(XCS-DIO-01)");
}

//--------------
// Destructor --
//--------------
DumpBld::~DumpBld ()
{
}

void
DumpBld::beginJob(Event& evt, Env& env)
{
  event(evt, env);
}

// Method which is called with event data
void 
DumpBld::event(Event& evt, Env& env)
{
  shared_ptr<Psana::Bld::BldDataEBeamV0> ebeam0 = evt.get(m_ebeamSrc);
  if (ebeam0.get()) {
    WithMsgLog(name(), info, str) {
      str << "Bld::BldDataEBeamV0:"
          << "\n  damageMask=" << ebeam0->damageMask()
          << "\n  ebeamCharge=" << ebeam0->ebeamCharge()
          << "\n  ebeamL3Energy=" << ebeam0->ebeamL3Energy()
          << "\n  ebeamLTUPosX=" << ebeam0->ebeamLTUPosX()
          << "\n  ebeamLTUPosY=" << ebeam0->ebeamLTUPosY()
          << "\n  ebeamLTUAngX=" << ebeam0->ebeamLTUAngX()
          << "\n  ebeamLTUAngY=" << ebeam0->ebeamLTUAngY();
    }
  }

  shared_ptr<Psana::Bld::BldDataEBeamV1> ebeam1 = evt.get(m_ebeamSrc);
  if (ebeam1.get()) {
    WithMsgLog(name(), info, str) {
      str << "Bld::BldDataEBeamV1:"
          << "\n  damageMask=" << ebeam1->damageMask()
          << "\n  ebeamCharge=" << ebeam1->ebeamCharge()
          << "\n  ebeamL3Energy=" << ebeam1->ebeamL3Energy()
          << "\n  ebeamLTUPosX=" << ebeam1->ebeamLTUPosX()
          << "\n  ebeamLTUPosY=" << ebeam1->ebeamLTUPosY()
          << "\n  ebeamLTUAngX=" << ebeam1->ebeamLTUAngX()
          << "\n  ebeamLTUAngY=" << ebeam1->ebeamLTUAngY()
          << "\n  ebeamPkCurrBC2=" << ebeam1->ebeamPkCurrBC2();
    }
  }

  shared_ptr<Psana::Bld::BldDataEBeamV2> ebeam2 = evt.get(m_ebeamSrc);
  if (ebeam2.get()) {
    WithMsgLog(name(), info, str) {
      str << "Bld::BldDataEBeamV2:"
          << "\n  damageMask=" << ebeam2->damageMask()
          << "\n  ebeamCharge=" << ebeam2->ebeamCharge()
          << "\n  ebeamL3Energy=" << ebeam2->ebeamL3Energy()
          << "\n  ebeamLTUPosX=" << ebeam2->ebeamLTUPosX()
          << "\n  ebeamLTUPosY=" << ebeam2->ebeamLTUPosY()
          << "\n  ebeamLTUAngX=" << ebeam2->ebeamLTUAngX()
          << "\n  ebeamLTUAngY=" << ebeam2->ebeamLTUAngY()
          << "\n  ebeamPkCurrBC2=" << ebeam2->ebeamPkCurrBC2()
          << "\n  ebeamEnergyBC2=" << ebeam2->ebeamEnergyBC2();
    }
  }

  shared_ptr<Psana::Bld::BldDataEBeamV3> ebeam3 = evt.get(m_ebeamSrc);
  if (ebeam3.get()) {
    WithMsgLog(name(), info, str) {
      str << "Bld::BldDataEBeamV3:"
          << "\n  damageMask=" << ebeam3->damageMask()
          << "\n  ebeamCharge=" << ebeam3->ebeamCharge()
          << "\n  ebeamL3Energy=" << ebeam3->ebeamL3Energy()
          << "\n  ebeamLTUPosX=" << ebeam3->ebeamLTUPosX()
          << "\n  ebeamLTUPosY=" << ebeam3->ebeamLTUPosY()
          << "\n  ebeamLTUAngX=" << ebeam3->ebeamLTUAngX()
          << "\n  ebeamLTUAngY=" << ebeam3->ebeamLTUAngY()
          << "\n  ebeamPkCurrBC2=" << ebeam3->ebeamPkCurrBC2()
          << "\n  ebeamEnergyBC2=" << ebeam3->ebeamEnergyBC2()
          << "\n  ebeamPkCurrBC1=" << ebeam3->ebeamPkCurrBC1()
          << "\n  ebeamEnergyBC1=" << ebeam3->ebeamEnergyBC1();
    }
  }

  shared_ptr<Psana::Bld::BldDataPhaseCavity> cav = evt.get(m_cavSrc);
  if (cav.get()) {
    WithMsgLog(name(), info, str) {
      str << "Bld::BldDataPhaseCavity:" 
          << "\n  fitTime1=" << cav->fitTime1()
          << "\n  fitTime2=" << cav->fitTime2()
          << "\n  charge1=" << cav->charge1()
          << "\n  charge2=" << cav->charge2();
    }
  }
  
  shared_ptr<Psana::Bld::BldDataFEEGasDetEnergy> fee = evt.get(m_feeSrc);
  if (fee.get()) {
    WithMsgLog(name(), info, str) {
      str << "Bld::BldDataFEEGasDetEnergy:"
          << "\n  f_11_ENRC=" << fee->f_11_ENRC()
          << "\n  f_12_ENRC=" << fee->f_12_ENRC()
          << "\n  f_21_ENRC=" << fee->f_21_ENRC()
          << "\n  f_22_ENRC=" << fee->f_22_ENRC();
    }
  }

  shared_ptr<Psana::Bld::BldDataIpimbV0> ipimb0 = evt.get(m_ipimbSrc);
  if (ipimb0.get()) {
    WithMsgLog(name(), info, str) {
      str << "Bld::BldDataIpimbV0:";
      const Psana::Ipimb::DataV1& ipimbData = ipimb0->ipimbData();
      str << "\n  Ipimb::DataV1:"
          << "\n    triggerCounter = " << ipimbData.triggerCounter()
          << "\n    config = " << ipimbData.config0()
          << "," << ipimbData.config1()
          << "," << ipimbData.config2()
          << "\n    channel = " << ipimbData.channel0()
          << "," << ipimbData.channel1()
          << "," << ipimbData.channel2()
          << "," << ipimbData.channel3()
          << "\n    volts = " << ipimbData.channel0Volts()
          << "," << ipimbData.channel1Volts()
          << "," << ipimbData.channel2Volts()
          << "," << ipimbData.channel3Volts()
          << "\n    checksum = " << ipimbData.checksum();
      
      const Psana::Ipimb::ConfigV1& ipimbConfig = ipimb0->ipimbConfig();
      str << "\n  Ipimb::ConfigV1:";
      str << "\n    triggerCounter = " << ipimbConfig.triggerCounter();
      str << "\n    serialID = " << ipimbConfig.serialID();
      str << "\n    chargeAmpRange = " << ipimbConfig.chargeAmpRange();
      str << "\n    calibrationRange = " << ipimbConfig.calibrationRange();
      str << "\n    resetLength = " << ipimbConfig.resetLength();
      str << "\n    resetDelay = " << ipimbConfig.resetDelay();
      str << "\n    chargeAmpRefVoltage = " << ipimbConfig.chargeAmpRefVoltage();
      str << "\n    calibrationVoltage = " << ipimbConfig.calibrationVoltage();
      str << "\n    diodeBias = " << ipimbConfig.diodeBias();
      str << "\n    status = " << ipimbConfig.status();
      str << "\n    errors = " << ipimbConfig.errors();
      str << "\n    calStrobeLength = " << ipimbConfig.calStrobeLength();
      str << "\n    trigDelay = " << ipimbConfig.trigDelay();
      
      const Psana::Lusi::IpmFexV1& ipmFexData = ipimb0->ipmFexData();
      str << "\n  Psana::Lusi::IpmFexV1:";
      str << "\n    sum = " << ipmFexData.sum();
      str << "\n    xpos = " << ipmFexData.xpos();
      str << "\n    ypos = " << ipmFexData.ypos();
      const ndarray<float, 1>& channel = ipmFexData.channel();
      str << "\n    channel =";
      for (unsigned i = 0; i < channel.shape()[0]; ++ i) {
        str << " " << channel[i];
      }
    }
  }

  shared_ptr<Psana::Bld::BldDataIpimbV1> ipimb1 = evt.get(m_ipimbSrc);
  if (ipimb1.get()) {
    WithMsgLog(name(), info, str) {
      str << "Bld::BldDataIpimbV1:";
      const Psana::Ipimb::DataV2& ipimbData = ipimb1->ipimbData();
      str << "\n  Ipimb::DataV1:"
          << "\n    triggerCounter = " << ipimbData.triggerCounter()
          << "\n    config = " << ipimbData.config0()
          << "," << ipimbData.config1()
          << "," << ipimbData.config2()
          << "\n    channel = " << ipimbData.channel0()
          << "," << ipimbData.channel1()
          << "," << ipimbData.channel2()
          << "," << ipimbData.channel3()
          << "\n    volts = " << ipimbData.channel0Volts()
          << "," << ipimbData.channel1Volts()
          << "," << ipimbData.channel2Volts()
          << "," << ipimbData.channel3Volts()
          << "\n    channel-ps = " << ipimbData.channel0ps()
          << "," << ipimbData.channel1ps()
          << "," << ipimbData.channel2ps()
          << "," << ipimbData.channel3ps()
          << "\n    volts-ps = " << ipimbData.channel0psVolts()
          << "," << ipimbData.channel1psVolts()
          << "," << ipimbData.channel2psVolts()
          << "," << ipimbData.channel3psVolts()
          << "\n    checksum = " << ipimbData.checksum();
      
      const Psana::Ipimb::ConfigV2& ipimbConfig = ipimb1->ipimbConfig();
      str << "\n  Ipimb::ConfigV1:";
      str << "\n    triggerCounter = " << ipimbConfig.triggerCounter();
      str << "\n    serialID = " << ipimbConfig.serialID();
      str << "\n    chargeAmpRange = " << ipimbConfig.chargeAmpRange();
      str << "\n    calibrationRange = " << ipimbConfig.calibrationRange();
      str << "\n    resetLength = " << ipimbConfig.resetLength();
      str << "\n    resetDelay = " << ipimbConfig.resetDelay();
      str << "\n    chargeAmpRefVoltage = " << ipimbConfig.chargeAmpRefVoltage();
      str << "\n    calibrationVoltage = " << ipimbConfig.calibrationVoltage();
      str << "\n    diodeBias = " << ipimbConfig.diodeBias();
      str << "\n    status = " << ipimbConfig.status();
      str << "\n    errors = " << ipimbConfig.errors();
      str << "\n    calStrobeLength = " << ipimbConfig.calStrobeLength();
      str << "\n    trigDelay = " << ipimbConfig.trigDelay();
      str << "\n    trigPsDelay = " << ipimbConfig.trigPsDelay();
      str << "\n    adcDelay = " << ipimbConfig.adcDelay();
      
      const Psana::Lusi::IpmFexV1& ipmFexData = ipimb1->ipmFexData();
      str << "\n  Psana::Lusi::IpmFexV1:";
      str << "\n    sum = " << ipmFexData.sum();
      str << "\n    xpos = " << ipmFexData.xpos();
      str << "\n    ypos = " << ipmFexData.ypos();
      const ndarray<float, 1>& channel = ipmFexData.channel();
      str << "\n    channel =";
      for (unsigned i = 0; i < channel.shape()[0]; ++ i) {
        str << " " << channel[i];
      }
    }
  }

  shared_ptr<Psana::Bld::BldDataPimV1> pim1 = evt.get(m_pimSrc);
  if (pim1.get()) {
    WithMsgLog(name(), info, str) {
      str << "Bld::BldDataPimV1:";
      const Psana::Pulnix::TM6740ConfigV2& camCfg = pim1->camConfig();
      str << "\n  Pulnix::TM6740ConfigV2:"
          << "\n    vref_a = " << camCfg.vref_a()
          << "\n    vref_b = " << camCfg.vref_b()
          << "\n    gain_a = " << camCfg.gain_a()
          << "\n    gain_b = " << camCfg.gain_b()
          << "\n    gain_balance = " << (camCfg.gain_balance() ? "yes" : "no")
          << "\n    output_resolution = " << camCfg.output_resolution()
          << "\n    output_resolution_bits = " << camCfg.output_resolution_bits()
          << "\n    horizontal_binning = " << camCfg.horizontal_binning()
          << "\n    vertical_binning = " << camCfg.vertical_binning()
          << "\n    lookuptable_mode = " << camCfg.lookuptable_mode();

      const Psana::Lusi::PimImageConfigV1& pimCfg = pim1->pimConfig();
      str << "\n  Lusi::PimImageConfigV1:"
          << "\n    xscale = " << pimCfg.xscale()
          << "\n    yscale = " << pimCfg.yscale();

      const Psana::Camera::FrameV1& frame = pim1->frame();
      str << "\n  Camera::FrameV1:"
          << "\n    width=" << frame.width()
          << "\n    height=" << frame.height()
          << "\n    depth=" << frame.depth()
          << "\n    offset=" << frame.offset();

      const ndarray<uint8_t, 2>& data8 = frame.data8();
      if (not data8.empty()) {
        str << "\n    data8=[" << int(data8[0][0])
            << ", " << int(data8[0][1])
            << ", " << int(data8[0][2]) << ", ...]";
      }

      const ndarray<uint16_t, 2>& data16 = frame.data16();
      if (not data16.empty()) {
        str << "\n    data16=[" << int(data16[0][0])
            << ", " << int(data16[0][1])
            << ", " << int(data16[0][2]) << ", ...]";
      }
    }
  }
}

} // namespace psana_examples
