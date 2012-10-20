//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class DumpIpimb...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana_examples/DumpIpimb.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
#include "psddl_psana/ipimb.ddl.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

using namespace psana_examples;
PSANA_MODULE_FACTORY(DumpIpimb)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana_examples {

//----------------
// Constructors --
//----------------
DumpIpimb::DumpIpimb (const std::string& name)
  : Module(name)
{
  m_src = configStr("source", "DetInfo(:Ipimb)");
}

//--------------
// Destructor --
//--------------
DumpIpimb::~DumpIpimb ()
{
}

// Method which is called at the beginning of the calibration cycle
void 
DumpIpimb::beginCalibCycle(Event& evt, Env& env)
{
  MsgLog(name(), info, "in beginCalibCycle()");

  shared_ptr<Psana::Ipimb::ConfigV1> config1 = env.configStore().get(m_src);
  if (config1.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "Ipimb::ConfigV1:";
      str << "\n  triggerCounter = " << config1->triggerCounter();
      str << "\n  serialID = " << config1->serialID();
      str << "\n  chargeAmpRange = " << config1->chargeAmpRange();
      str << "\n  diodeGain = " << config1->diodeGain(0) << " " << config1->diodeGain(1) 
          << " " << config1->diodeGain(2) << " " << config1->diodeGain(3);
      str << "\n  calibrationRange = " << config1->calibrationRange();
      str << "\n  resetLength = " << config1->resetLength();
      str << "\n  resetDelay = " << config1->resetDelay();
      str << "\n  chargeAmpRefVoltage = " << config1->chargeAmpRefVoltage();
      str << "\n  calibrationVoltage = " << config1->calibrationVoltage();
      str << "\n  diodeBias = " << config1->diodeBias();
      str << "\n  status = " << config1->status();
      str << "\n  errors = " << config1->errors();
      str << "\n  calStrobeLength = " << config1->calStrobeLength();
      str << "\n  trigDelay = " << config1->trigDelay();
    }
    
  }

  shared_ptr<Psana::Ipimb::ConfigV2> config2 = env.configStore().get(m_src);
  if (config2.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "Ipimb::ConfigV2:";
      str << "\n  triggerCounter = " << config2->triggerCounter();
      str << "\n  serialID = " << config2->serialID();
      str << "\n  chargeAmpRange = " << config2->chargeAmpRange();
      str << "\n  diodeGain = " << config2->diodeGain(0) << " " << config2->diodeGain(1) 
          << " " << config2->diodeGain(2) << " " << config2->diodeGain(3);
      str << "\n  calibrationRange = " << config2->calibrationRange();
      str << "\n  resetLength = " << config2->resetLength();
      str << "\n  resetDelay = " << config2->resetDelay();
      str << "\n  chargeAmpRefVoltage = " << config2->chargeAmpRefVoltage();
      str << "\n  calibrationVoltage = " << config2->calibrationVoltage();
      str << "\n  diodeBias = " << config2->diodeBias();
      str << "\n  status = " << config2->status();
      str << "\n  errors = " << config2->errors();
      str << "\n  calStrobeLength = " << config2->calStrobeLength();
      str << "\n  trigDelay = " << config2->trigDelay();
      str << "\n  trigPsDelay = " << config2->trigPsDelay();
      str << "\n  adcDelay = " << config2->adcDelay();
    }
    
  }

}

// Method which is called with event data
void 
DumpIpimb::event(Event& evt, Env& env)
{
  shared_ptr<Psana::Ipimb::DataV1> data1 = evt.get(m_src);
  if (data1.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "Ipimb::DataV1:"
          << "\n  triggerCounter = " << data1->triggerCounter()
          << "\n  config = " << data1->config0()
          << "," << data1->config1()
          << "," << data1->config2()
          << "\n  channel = " << data1->channel0()
          << "," << data1->channel1()
          << "," << data1->channel2()
          << "," << data1->channel3()
          << "\n  volts = " << data1->channel0Volts()
          << "," << data1->channel1Volts()
          << "," << data1->channel2Volts()
          << "," << data1->channel3Volts()
          << "\n  checksum = " << data1->checksum();
    }
  }

  shared_ptr<Psana::Ipimb::DataV2> data2 = evt.get(m_src);
  if (data2.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "Ipimb::DataV2:"
          << "\n  triggerCounter = " << data2->triggerCounter()
          << "\n  config = " << data2->config0()
          << "," << data2->config1()
          << "," << data2->config2()
          << "\n  channel = " << data2->channel0()
          << "," << data2->channel1()
          << "," << data2->channel2()
          << "," << data2->channel3()
          << "\n  volts = " << data2->channel0Volts()
          << "," << data2->channel1Volts()
          << "," << data2->channel2Volts()
          << "," << data2->channel3Volts()
          << "\n  channel-ps = " << data2->channel0ps()
          << "," << data2->channel1ps()
          << "," << data2->channel2ps()
          << "," << data2->channel3ps()
          << "\n  volts-ps = " << data2->channel0psVolts()
          << "," << data2->channel1psVolts()
          << "," << data2->channel2psVolts()
          << "," << data2->channel3psVolts()
          << "\n  checksum = " << data2->checksum();
    }
  }
}
  
} // namespace psana_examples
