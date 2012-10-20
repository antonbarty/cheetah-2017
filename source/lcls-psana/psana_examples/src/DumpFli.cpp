//--------------------------------------------------------------------------
// File and Version Information:
//      $Id: DumpFli.cpp 3359 2012-05-09 12:04:08Z ofte@SLAC.STANFORD.EDU $
//
// Description:
//	Class DumpFli...
//
// Author List:
//      Ingrid Ofte
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana_examples/DumpFli.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
#include "psddl_psana/fli.ddl.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

using namespace psana_examples;
PSANA_MODULE_FACTORY(DumpFli)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana_examples {

//----------------
// Constructors --
//----------------
DumpFli::DumpFli (const std::string& name)
  : Module(name)
{
  m_src = configStr("source", "DetInfo(:Fli)");
}

//--------------
// Destructor --
//--------------
DumpFli::~DumpFli ()
{
}

// Method which is called at the beginning of the calibration cycle
void 
DumpFli::beginCalibCycle(Event& evt, Env& env)
{
  MsgLog(name(), trace, "in beginCalibCycle()");

  shared_ptr<Psana::Fli::ConfigV1> config1 = env.configStore().get(m_src);
  if (config1.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "Fli::ConfigV1:";
      str << "\n  width = " << config1->width();
      str << "\n  height = " << config1->height();
      str << "\n  orgX = " << config1->orgX();
      str << "\n  orgY = " << config1->orgY();
      str << "\n  binX = " << config1->binX();
      str << "\n  binY = " << config1->binY();
      str << "\n  exposureEventCode = " << config1->exposureEventCode();
      str << "\n  exposureTime = " << config1->exposureTime();
      str << "\n  coolingTemp = " << config1->coolingTemp();
      str << "\n  gainIndex = " << config1->gainIndex();
      str << "\n  readoutSpeedIndex = " << config1->readoutSpeedIndex();
      str << "\n  numDelayShots = " << config1->numDelayShots();
      str << "\n  frameSize = " << config1->frameSize();
      str << "\n  numPixels = " << config1->numPixels();
      str << "\n  numPixelsX = " << config1->numPixelsX();
      str << "\n  numPixelsY = " << config1->numPixelsY();
    }
    
  }

}

// Method which is called with event data
void 
DumpFli::event(Event& evt, Env& env)
{
  shared_ptr<Psana::Fli::FrameV1> frame = evt.get(m_src);
  if (frame.get()) {
    WithMsgLog(name(), info, str) {
      str << "Fli::FrameV1:";
      str << "\n  shotIdStart = " << frame->shotIdStart();
      str << "\n  readoutTime = " << frame->readoutTime();
      str << "\n  temprature = " << frame->temperature();

      const ndarray<uint16_t, 2>& data = frame->data();
      str << "\n  data =";
      for (int i = 0; i < 10; ++ i) {
        str << " " << data[0][i];
      }
      str << " ...";
    }
  }
}
  
} // namespace psana_examples
