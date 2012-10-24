//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class DumpPulnix...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana_examples/DumpPulnix.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
#include "psddl_psana/pulnix.ddl.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

using namespace psana_examples;
PSANA_MODULE_FACTORY(DumpPulnix)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana_examples {

//----------------
// Constructors --
//----------------
DumpPulnix::DumpPulnix (const std::string& name)
  : Module(name)
{
  m_src = configStr("source", "DetInfo(:Tm6740)");
}

//--------------
// Destructor --
//--------------
DumpPulnix::~DumpPulnix ()
{
}

// Method which is called at the beginning of the calibration cycle
void 
DumpPulnix::beginCalibCycle(Event& evt, Env& env)
{
  MsgLog(name(), trace, "in beginCalibCycle()");

  shared_ptr<Psana::Pulnix::TM6740ConfigV1> config1 = env.configStore().get(m_src);
  if (config1.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "Pulnix::TM6740ConfigV1:";
      str << "\n  gain_a = " << config1->gain_a();
      str << "\n  gain_b = " << config1->gain_b();
      str << "\n  vref = " << config1->vref();
      str << "\n  shutter_width = " << config1->shutter_width();
      str << "\n  gain_balance = " << int(config1->gain_balance());
      str << "\n  output_resolution = " << config1->output_resolution();
      str << "\n  horizontal_binning = " << config1->horizontal_binning();
      str << "\n  vertical_binning = " << config1->vertical_binning();
      str << "\n  lookuptable_mode = " << config1->lookuptable_mode();
      str << "\n  output_resolution_bits = " << int(config1->output_resolution_bits());
    }
    
  }

  shared_ptr<Psana::Pulnix::TM6740ConfigV2> config2 = env.configStore().get(m_src);
  if (config2.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "Pulnix::TM6740ConfigV2:";
      str << "\n  gain_a = " << config2->gain_a();
      str << "\n  gain_b = " << config2->gain_b();
      str << "\n  vref_a = " << config2->vref_a();
      str << "\n  vref_b = " << config2->vref_b();
      str << "\n  gain_balance = " << int(config2->gain_balance());
      str << "\n  output_resolution = " << config2->output_resolution();
      str << "\n  horizontal_binning = " << config2->horizontal_binning();
      str << "\n  vertical_binning = " << config2->vertical_binning();
      str << "\n  lookuptable_mode = " << config2->lookuptable_mode();
      str << "\n  output_resolution_bits = " << int(config2->output_resolution_bits());
    }
    
  }
}

// Method which is called with event data
void 
DumpPulnix::event(Event& evt, Env& env)
{
}
  
} // namespace psana_examples
