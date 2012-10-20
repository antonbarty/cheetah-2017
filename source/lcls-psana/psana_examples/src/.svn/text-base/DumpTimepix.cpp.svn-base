//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class DumpTimepix...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana_examples/DumpTimepix.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
// to work with detector data include corresponding 
// header from psddl_psana package
#include "psddl_psana/timepix.ddl.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

// This declares this class as psana module
using namespace psana_examples;
PSANA_MODULE_FACTORY(DumpTimepix)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana_examples {

//----------------
// Constructors --
//----------------
DumpTimepix::DumpTimepix (const std::string& name)
  : Module(name)
  , m_src()
{
  // get the values from configuration or use defaults
  m_src = configStr("source", "DetInfo(:Timepix)");
}

//--------------
// Destructor --
//--------------
DumpTimepix::~DumpTimepix ()
{
}

/// Method which is called at the beginning of the calibration cycle
void 
DumpTimepix::beginCalibCycle(Event& evt, Env& env)
{
  MsgLog(name(), trace, "in beginCalibCycle()");

  shared_ptr<Psana::Timepix::ConfigV1> config1 = env.configStore().get(m_src);
  if (config1.get()) {

    WithMsgLog(name(), info, str) {
      str << "Timepix::ConfigV1:";

      str << "\n  readoutSpeed = " << int(config1->readoutSpeed());
      str << "\n  triggerMode = " << int(config1->triggerMode());
      str << "\n  shutterTimeout = " << config1->shutterTimeout();
      str << "\n  dac0Ikrum = " << config1->dac0Ikrum();
      str << "\n  dac0Disc = " << config1->dac0Disc();
      str << "\n  dac0Preamp = " << config1->dac0Preamp();
      str << "\n  dac0BufAnalogA = " << config1->dac0BufAnalogA();
      str << "\n  dac0BufAnalogB = " << config1->dac0BufAnalogB();
      str << "\n  dac0Hist = " << config1->dac0Hist();
      str << "\n  dac0ThlFine = " << config1->dac0ThlFine();
      str << "\n  dac0ThlCourse = " << config1->dac0ThlCourse();
      str << "\n  dac0Vcas = " << config1->dac0Vcas();
      str << "\n  dac0Fbk = " << config1->dac0Fbk();
      str << "\n  dac0Gnd = " << config1->dac0Gnd();
      str << "\n  dac0Ths = " << config1->dac0Ths();
      str << "\n  dac0BiasLvds = " << config1->dac0BiasLvds();
      str << "\n  dac0RefLvds = " << config1->dac0RefLvds();
      str << "\n  dac1Ikrum = " << config1->dac1Ikrum();
      str << "\n  dac1Disc = " << config1->dac1Disc();
      str << "\n  dac1Preamp = " << config1->dac1Preamp();
      str << "\n  dac1BufAnalogA = " << config1->dac1BufAnalogA();
      str << "\n  dac1BufAnalogB = " << config1->dac1BufAnalogB();
      str << "\n  dac1Hist = " << config1->dac1Hist();
      str << "\n  dac1ThlFine = " << config1->dac1ThlFine();
      str << "\n  dac1ThlCourse = " << config1->dac1ThlCourse();
      str << "\n  dac1Vcas = " << config1->dac1Vcas();
      str << "\n  dac1Fbk = " << config1->dac1Fbk();
      str << "\n  dac1Gnd = " << config1->dac1Gnd();
      str << "\n  dac1Ths = " << config1->dac1Ths();
      str << "\n  dac1BiasLvds = " << config1->dac1BiasLvds();
      str << "\n  dac1RefLvds = " << config1->dac1RefLvds();
      str << "\n  dac2Ikrum = " << config1->dac2Ikrum();
      str << "\n  dac2Disc = " << config1->dac2Disc();
      str << "\n  dac2Preamp = " << config1->dac2Preamp();
      str << "\n  dac2BufAnalogA = " << config1->dac2BufAnalogA();
      str << "\n  dac2BufAnalogB = " << config1->dac2BufAnalogB();
      str << "\n  dac2Hist = " << config1->dac2Hist();
      str << "\n  dac2ThlFine = " << config1->dac2ThlFine();
      str << "\n  dac2ThlCourse = " << config1->dac2ThlCourse();
      str << "\n  dac2Vcas = " << config1->dac2Vcas();
      str << "\n  dac2Fbk = " << config1->dac2Fbk();
      str << "\n  dac2Gnd = " << config1->dac2Gnd();
      str << "\n  dac2Ths = " << config1->dac2Ths();
      str << "\n  dac2BiasLvds = " << config1->dac2BiasLvds();
      str << "\n  dac2RefLvds = " << config1->dac2RefLvds();
      str << "\n  dac3Ikrum = " << config1->dac3Ikrum();
      str << "\n  dac3Disc = " << config1->dac3Disc();
      str << "\n  dac3Preamp = " << config1->dac3Preamp();
      str << "\n  dac3BufAnalogA = " << config1->dac3BufAnalogA();
      str << "\n  dac3BufAnalogB = " << config1->dac3BufAnalogB();
      str << "\n  dac3Hist = " << config1->dac3Hist();
      str << "\n  dac3ThlFine = " << config1->dac3ThlFine();
      str << "\n  dac3ThlCourse = " << config1->dac3ThlCourse();
      str << "\n  dac3Vcas = " << config1->dac3Vcas();
      str << "\n  dac3Fbk = " << config1->dac3Fbk();
      str << "\n  dac3Gnd = " << config1->dac3Gnd();
      str << "\n  dac3Ths = " << config1->dac3Ths();
      str << "\n  dac3BiasLvds = " << config1->dac3BiasLvds();
      str << "\n  dac3RefLvds = " << config1->dac3RefLvds();
    }

  }

  shared_ptr<Psana::Timepix::ConfigV2> config2 = env.configStore().get(m_src);
  if (config2.get()) {

    WithMsgLog(name(), info, str) {
      str << "Timepix::ConfigV2:";

      str << "\n  readoutSpeed = " << int(config2->readoutSpeed());
      str << "\n  triggerMode = " << int(config2->triggerMode());
      str << "\n  timepixSpeed = " << config2->timepixSpeed();
      str << "\n  dac0Ikrum = " << config2->dac0Ikrum();
      str << "\n  dac0Disc = " << config2->dac0Disc();
      str << "\n  dac0Preamp = " << config2->dac0Preamp();
      str << "\n  dac0BufAnalogA = " << config2->dac0BufAnalogA();
      str << "\n  dac0BufAnalogB = " << config2->dac0BufAnalogB();
      str << "\n  dac0Hist = " << config2->dac0Hist();
      str << "\n  dac0ThlFine = " << config2->dac0ThlFine();
      str << "\n  dac0ThlCourse = " << config2->dac0ThlCourse();
      str << "\n  dac0Vcas = " << config2->dac0Vcas();
      str << "\n  dac0Fbk = " << config2->dac0Fbk();
      str << "\n  dac0Gnd = " << config2->dac0Gnd();
      str << "\n  dac0Ths = " << config2->dac0Ths();
      str << "\n  dac0BiasLvds = " << config2->dac0BiasLvds();
      str << "\n  dac0RefLvds = " << config2->dac0RefLvds();
      str << "\n  dac1Ikrum = " << config2->dac1Ikrum();
      str << "\n  dac1Disc = " << config2->dac1Disc();
      str << "\n  dac1Preamp = " << config2->dac1Preamp();
      str << "\n  dac1BufAnalogA = " << config2->dac1BufAnalogA();
      str << "\n  dac1BufAnalogB = " << config2->dac1BufAnalogB();
      str << "\n  dac1Hist = " << config2->dac1Hist();
      str << "\n  dac1ThlFine = " << config2->dac1ThlFine();
      str << "\n  dac1ThlCourse = " << config2->dac1ThlCourse();
      str << "\n  dac1Vcas = " << config2->dac1Vcas();
      str << "\n  dac1Fbk = " << config2->dac1Fbk();
      str << "\n  dac1Gnd = " << config2->dac1Gnd();
      str << "\n  dac1Ths = " << config2->dac1Ths();
      str << "\n  dac1BiasLvds = " << config2->dac1BiasLvds();
      str << "\n  dac1RefLvds = " << config2->dac1RefLvds();
      str << "\n  dac2Ikrum = " << config2->dac2Ikrum();
      str << "\n  dac2Disc = " << config2->dac2Disc();
      str << "\n  dac2Preamp = " << config2->dac2Preamp();
      str << "\n  dac2BufAnalogA = " << config2->dac2BufAnalogA();
      str << "\n  dac2BufAnalogB = " << config2->dac2BufAnalogB();
      str << "\n  dac2Hist = " << config2->dac2Hist();
      str << "\n  dac2ThlFine = " << config2->dac2ThlFine();
      str << "\n  dac2ThlCourse = " << config2->dac2ThlCourse();
      str << "\n  dac2Vcas = " << config2->dac2Vcas();
      str << "\n  dac2Fbk = " << config2->dac2Fbk();
      str << "\n  dac2Gnd = " << config2->dac2Gnd();
      str << "\n  dac2Ths = " << config2->dac2Ths();
      str << "\n  dac2BiasLvds = " << config2->dac2BiasLvds();
      str << "\n  dac2RefLvds = " << config2->dac2RefLvds();
      str << "\n  dac3Ikrum = " << config2->dac3Ikrum();
      str << "\n  dac3Disc = " << config2->dac3Disc();
      str << "\n  dac3Preamp = " << config2->dac3Preamp();
      str << "\n  dac3BufAnalogA = " << config2->dac3BufAnalogA();
      str << "\n  dac3BufAnalogB = " << config2->dac3BufAnalogB();
      str << "\n  dac3Hist = " << config2->dac3Hist();
      str << "\n  dac3ThlFine = " << config2->dac3ThlFine();
      str << "\n  dac3ThlCourse = " << config2->dac3ThlCourse();
      str << "\n  dac3Vcas = " << config2->dac3Vcas();
      str << "\n  dac3Fbk = " << config2->dac3Fbk();
      str << "\n  dac3Gnd = " << config2->dac3Gnd();
      str << "\n  dac3Ths = " << config2->dac3Ths();
      str << "\n  dac3BiasLvds = " << config2->dac3BiasLvds();
      str << "\n  dac3RefLvds = " << config2->dac3RefLvds();
      str << "\n  chipCount = " << config2->chipCount();
      str << "\n  driverVersion = " << config2->driverVersion();
      str << "\n  firmwareVersion = " << config2->firmwareVersion();
      str << "\n  pixelThreshSize = " << config2->pixelThreshSize();
      const ndarray<uint8_t, 1>& pixelThresh = config2->pixelThresh();
      str << "\n  pixelThresh = ";
      for (unsigned i = 0; i < config2->pixelThreshSize() and i < 20; ++ i) {
        str << int(pixelThresh[i]) << ' ';
      }
      str << "...";
      str << "\n  chip names = " << config2->chip0Name() << " " << config2->chip1Name() << " " << config2->chip2Name() << " " << config2->chip3Name();
      str << "\n  chip IDs   = " << config2->chip0ID() << " " << config2->chip1ID() << " " << config2->chip2ID() << " " << config2->chip3ID();

    }

  }
}

/// Method which is called with event data, this is the only required 
/// method, all other methods are optional
void 
DumpTimepix::event(Event& evt, Env& env)
{

  // ==============================================================
  //  Psana transparently replaces Timepix::DataV1 objects with
  //  Timepix::DataV2, so the code for Timepix::DataV1 is not likely
  //  to be executed at all.
  // ==============================================================
  shared_ptr<Psana::Timepix::DataV1> data1 = evt.get(m_src);
  if (data1.get()) {
    WithMsgLog(name(), info, str) {
      str << "Timepix::DataV1:";

      str << "\n  timestamp = " << data1->timestamp();
      str << "\n  frameCounter = " << data1->frameCounter();
      str << "\n  lostRows = " << data1->lostRows();

      const ndarray<uint16_t, 2>& img = data1->data();
      str << "\n  data =";
      str << " (" << img.shape()[0] << ", " << img.shape()[0] << ")";
      for (int i = 0; i < 10; ++ i) {
        str << " " << img[0][i];
      }
      str << " ...";
    }
  }

  shared_ptr<Psana::Timepix::DataV2> data2 = evt.get(m_src);
  if (data2.get()) {
    WithMsgLog(name(), info, str) {
      str << "Timepix::DataV2:";

      str << "\n  timestamp = " << data2->timestamp();
      str << "\n  frameCounter = " << data2->frameCounter();
      str << "\n  lostRows = " << data2->lostRows();

      const ndarray<uint16_t, 2>& img = data2->data();
      str << "\n  data =";
      str << " (" << img.shape()[0] << ", " << img.shape()[0] << ")";
      for (int i = 0; i < 10; ++ i) {
        str << " " << img[0][i];
      }
      str << " ...";
    }
  }
}
  

} // namespace psana_examples
