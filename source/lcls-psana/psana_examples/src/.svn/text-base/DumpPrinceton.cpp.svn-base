//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class DumpPrinceton...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana_examples/DumpPrinceton.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
#include "psddl_psana/princeton.ddl.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

using namespace psana_examples;
PSANA_MODULE_FACTORY(DumpPrinceton)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana_examples {

//----------------
// Constructors --
//----------------
DumpPrinceton::DumpPrinceton (const std::string& name)
  : Module(name)
{
  m_src = configStr("source", "DetInfo(:Princeton)");
}

//--------------
// Destructor --
//--------------
DumpPrinceton::~DumpPrinceton ()
{
}

// Method which is called at the beginning of the calibration cycle
void 
DumpPrinceton::beginCalibCycle(Event& evt, Env& env)
{
  MsgLog(name(), trace, "in beginCalibCycle()");

  shared_ptr<Psana::Princeton::ConfigV1> config1 = env.configStore().get(m_src);
  if (config1.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "Princeton::ConfigV1:";
      str << "\n  width = " << config1->width();
      str << "\n  height = " << config1->height();
      str << "\n  orgX = " << config1->orgX();
      str << "\n  orgY = " << config1->orgY();
      str << "\n  binX = " << config1->binX();
      str << "\n  binY = " << config1->binY();
      str << "\n  exposureTime = " << config1->exposureTime();
      str << "\n  coolingTemp = " << config1->coolingTemp();
      str << "\n  readoutSpeedIndex = " << config1->readoutSpeedIndex();
      str << "\n  readoutEventCode = " << config1->readoutEventCode();
      str << "\n  delayMode = " << config1->delayMode();
      str << "\n  frameSize = " << config1->frameSize();
      str << "\n  numPixels = " << config1->numPixels();
    }
    
  }

  shared_ptr<Psana::Princeton::ConfigV2> config2 = env.configStore().get(m_src);
  if (config2.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "Princeton::ConfigV2:";
      str << "\n  width = " << config2->width();
      str << "\n  height = " << config2->height();
      str << "\n  orgX = " << config2->orgX();
      str << "\n  orgY = " << config2->orgY();
      str << "\n  binX = " << config2->binX();
      str << "\n  binY = " << config2->binY();
      str << "\n  exposureTime = " << config2->exposureTime();
      str << "\n  coolingTemp = " << config2->coolingTemp();
      str << "\n  gainIndex = " << config2->gainIndex();
      str << "\n  readoutSpeedIndex = " << config2->readoutSpeedIndex();
      str << "\n  readoutEventCode = " << config2->readoutEventCode();
      str << "\n  delayMode = " << config2->delayMode();
      str << "\n  frameSize = " << config2->frameSize();
      str << "\n  numPixels = " << config2->numPixels();
    }
    
  }

  shared_ptr<Psana::Princeton::ConfigV3> config3 = env.configStore().get(m_src);
  if (config3.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "Princeton::ConfigV2:";
      str << "\n  width = " << config3->width();
      str << "\n  height = " << config3->height();
      str << "\n  orgX = " << config3->orgX();
      str << "\n  orgY = " << config3->orgY();
      str << "\n  binX = " << config3->binX();
      str << "\n  binY = " << config3->binY();
      str << "\n  exposureTime = " << config3->exposureTime();
      str << "\n  coolingTemp = " << config3->coolingTemp();
      str << "\n  gainIndex = " << config3->gainIndex();
      str << "\n  readoutSpeedIndex = " << config3->readoutSpeedIndex();
      str << "\n  exposureEventCode = " << config3->exposureEventCode();
      str << "\n  numDelayShots = " << config3->numDelayShots();
      str << "\n  frameSize = " << config3->frameSize();
      str << "\n  numPixels = " << config3->numPixels();
    }
    
  }
}

// Method which is called with event data
void 
DumpPrinceton::event(Event& evt, Env& env)
{
  shared_ptr<Psana::Princeton::FrameV1> frame = evt.get(m_src);
  if (frame.get()) {
    WithMsgLog(name(), info, str) {
      str << "Princeton::FrameV1:";
      str << "\n  shotIdStart = " << frame->shotIdStart();
      str << "\n  readoutTime = " << frame->readoutTime();

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
