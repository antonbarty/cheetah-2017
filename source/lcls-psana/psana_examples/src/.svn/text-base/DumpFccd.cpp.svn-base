//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class DumpFccd...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana_examples/DumpFccd.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
#include "psddl_psana/fccd.ddl.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

using namespace psana_examples;
PSANA_MODULE_FACTORY(DumpFccd)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana_examples {

//----------------
// Constructors --
//----------------
DumpFccd::DumpFccd (const std::string& name)
  : Module(name)
{
  m_src = configStr("source", "DetInfo(:Fccd)");
}

//--------------
// Destructor --
//--------------
DumpFccd::~DumpFccd ()
{
}

// Method which is called at the beginning of the calibration cycle
void 
DumpFccd::beginCalibCycle(Event& evt, Env& env)
{
  MsgLog(name(), trace, "in beginCalibCycle()");

  shared_ptr<Psana::FCCD::FccdConfigV1> config1 = env.configStore().get(m_src);
  if (config1.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "FCCD::FccdConfigV1:";
      str << "\n  outputMode = " << config1->outputMode();
      str << "\n  width = " << config1->width();
      str << "\n  height = " << config1->height();
      str << "\n  trimmedWidth = " << config1->trimmedWidth();
      str << "\n  trimmedHeight = " << config1->trimmedHeight();
    }
    
  }

  shared_ptr<Psana::FCCD::FccdConfigV2> config2 = env.configStore().get(m_src);
  if (config2.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "FCCD::FccdConfigV2:";
      str << "\n  outputMode = " << config2->outputMode();
      str << "\n  ccdEnable = " << int(config2->ccdEnable());
      str << "\n  focusMode = " << int(config2->focusMode());
      str << "\n  exposureTime = " << config2->exposureTime();
      str << "\n  dacVoltages = [" << config2->dacVoltages()[0]
          << " " << config2->dacVoltages()[1] << " ...]";
      str << "\n  waveforms = [" << config2->waveforms()[0]
          << " " << config2->waveforms()[1] << " ...]";
      str << "\n  width = " << config2->width();
      str << "\n  height = " << config2->height();
      str << "\n  trimmedWidth = " << config2->trimmedWidth();
      str << "\n  trimmedHeight = " << config2->trimmedHeight();
    }
    
  }
}

// Method which is called with event data
void 
DumpFccd::event(Event& evt, Env& env)
{
}
  
} // namespace psana_examples
