//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class DumpPnccd...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana_examples/DumpPnccd.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
#include "psddl_psana/pnccd.ddl.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

using namespace psana_examples;
PSANA_MODULE_FACTORY(DumpPnccd)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana_examples {

//----------------
// Constructors --
//----------------
DumpPnccd::DumpPnccd (const std::string& name)
  : Module(name)
{
    m_src = configStr("source", "DetInfo(:pnCCD)");
}

//--------------
// Destructor --
//--------------
DumpPnccd::~DumpPnccd ()
{
}

// Method which is called at the beginning of the calibration cycle
void 
DumpPnccd::beginCalibCycle(Event& evt, Env& env)
{
  MsgLog(name(), trace, "in beginCalibCycle()");

  shared_ptr<Psana::PNCCD::ConfigV1> config1 = env.configStore().get(m_src);
  if (config1.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "PNCCD::ConfigV1:";
      str << "\n  numLinks = " << config1->numLinks();
      str << "\n  payloadSizePerLink = " << config1->payloadSizePerLink();
    }
    
  }

  shared_ptr<Psana::PNCCD::ConfigV2> config2 = env.configStore().get(m_src);
  if (config2.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "PNCCD::ConfigV2:";
      str << "\n  numLinks = " << config2->numLinks();
      str << "\n  payloadSizePerLink = " << config2->payloadSizePerLink();
      str << "\n  numChannels = " << config2->numChannels();
      str << "\n  numRows = " << config2->numRows();
      str << "\n  numSubmoduleChannels = " << config2->numSubmoduleChannels();
      str << "\n  numSubmoduleRows = " << config2->numSubmoduleRows();
      str << "\n  numSubmodules = " << config2->numSubmodules();
      str << "\n  camexMagic = " << config2->camexMagic();
      str << "\n  info = " << config2->info();
      str << "\n  timingFName = " << config2->timingFName();
    }
    
  }

}

// Method which is called with event data
void 
DumpPnccd::event(Event& evt, Env& env)
{
  // First dump raw frames as they come out of device
  shared_ptr<Psana::PNCCD::FramesV1> data = evt.get(m_src);
  if (data) {
    WithMsgLog(name(), info, str) {

      str << "PNCCD::FramesV1:";

      for (unsigned i = 0 ; i != data->numLinks(); ++ i) {

        const Psana::PNCCD::FrameV1& frame = data->frame(i);

        str << "\n  Frame #" << i;

        str << "\n    specialWord = " << frame.specialWord();
        str << "\n    frameNumber = " << frame.frameNumber();
        str << "\n    timeStampHi = " << frame.timeStampHi();
        str << "\n    timeStampLo = " << frame.timeStampLo();

        const ndarray<uint16_t, 2> data = frame.data();
        str << "\n    frame size = " << data.shape()[0] << 'x' << data.shape()[1];
        str << "\n    data =";
        for (int i = 0; i < 10; ++ i) {
          str << " " << data[0][i];
        }
        str << " ...";
      }
    }
  }

  // Next dump combined full frame
  shared_ptr<Psana::PNCCD::FullFrameV1> frame = evt.get(m_src);
  if (frame) {
    WithMsgLog(name(), info, str) {

      str << "PNCCD::FullFrameV1:";
      str << "\n    specialWord = " << frame->specialWord();
      str << "\n    frameNumber = " << frame->frameNumber();
      str << "\n    timeStampHi = " << frame->timeStampHi();
      str << "\n    timeStampLo = " << frame->timeStampLo();

      const ndarray<uint16_t, 2> data = frame->data();
      str << "\n    frame size = " << data.shape()[0] << 'x' << data.shape()[1];
      str << "\n    data =";
      for (int i = 0; i < 10; ++ i) {
        str << " " << data[0][i];
      }
      str << " ...";
    }

  }

}

} // namespace psana_examples
