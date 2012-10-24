//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class DumpAcqiris...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana_examples/DumpAcqiris.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <algorithm>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
#include "pdsdata/xtc/DetInfo.hh"
#include "psddl_psana/acqiris.ddl.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

using namespace psana_examples;
PSANA_MODULE_FACTORY(DumpAcqiris)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana_examples {

//----------------
// Constructors --
//----------------
DumpAcqiris::DumpAcqiris (const std::string& name)
  : Module(name)
{
}

//--------------
// Destructor --
//--------------
DumpAcqiris::~DumpAcqiris ()
{
}

// Method which is called at the beginning of the calibration cycle
void 
DumpAcqiris::beginCalibCycle(Event& evt, Env& env)
{
  MsgLog(name(), trace, "in beginCalibCycle()");

  Source src = configStr("source", "DetInfo(:Acqiris)");
  shared_ptr<Psana::Acqiris::ConfigV1> acqConfig = env.configStore().get(src, &m_src);
  if (acqConfig.get()) {
    WithMsgLog(name(), info, str) {
      str << "Acqiris::ConfigV1: nbrBanks=" << acqConfig->nbrBanks()
          << " channelMask=" << acqConfig->channelMask()
          << " nbrChannels=" << acqConfig->nbrChannels()
          << " nbrConvertersPerChannel=" << acqConfig->nbrConvertersPerChannel();
     
      const Psana::Acqiris::HorizV1& h = acqConfig->horiz();
      str << "\n  horiz: sampInterval=" << h.sampInterval()
           << " delayTime=" << h.delayTime()
           << " nbrSegments=" << h.nbrSegments()
           << " nbrSamples=" << h.nbrSamples();
      
      const ndarray<Psana::Acqiris::VertV1, 1>& vert = acqConfig->vert();
      for (unsigned ch = 0; ch < vert.shape()[0]; ++ ch) {
        const Psana::Acqiris::VertV1& v = vert[ch];
        str << "\n  vert(" << ch << "):"
            << " fullScale=" << v.fullScale()
            << " slope=" << v.slope()
            << " offset=" << v.offset()
            << " coupling=" << v.coupling()
            << " bandwidth=" << v.bandwidth();
      }
    }
  }
}

// Method which is called with event data
void 
DumpAcqiris::event(Event& evt, Env& env)
{

  Pds::Src src;
  shared_ptr<Psana::Acqiris::DataDescV1> acqData = evt.get(m_src);
  if (acqData.get()) {
    
    // find matching config object
    shared_ptr<Psana::Acqiris::ConfigV1> acqConfig = env.configStore().get(m_src);
    
    // loop over channels
    int nchan = acqData->data_shape()[0];
    for (int chan = 0; chan < nchan; ++ chan) {
      
      const Psana::Acqiris::DataDescV1Elem& elem = acqData->data(chan);

      const Psana::Acqiris::VertV1& v = acqConfig->vert()[chan];
      double slope = v.slope();
      double offset = v.offset();

      WithMsgLog(name(), info, str ) {

        str << "Acqiris::DataDescV1: channel=" << chan
           << "\n  nbrSegments=" << elem.nbrSegments()
           << "\n  nbrSamplesInSeg=" << elem.nbrSamplesInSeg()
           << "\n  indexFirstPoint=" << elem.indexFirstPoint();
        
        const ndarray<Psana::Acqiris::TimestampV1, 1>& timestamps = elem.timestamp();
        const ndarray<int16_t, 2>& waveforms = elem.waveforms();

        // loop over segments
        for (unsigned seg = 0; seg < elem.nbrSegments(); ++ seg) {
          
          str << "\n  Segment #" << seg
              << "\n    timestamp=" << timestamps[seg].pos()
              << "\n    data=[";
          
          unsigned size = std::min(elem.nbrSamplesInSeg(), 32U);
          for (unsigned i = 0; i < size; ++ i) {
            str << (waveforms[seg][i]*slope + offset) << ", ";
          }
          str << "...]";
        
        }
      }
    }
  }
}

} // namespace psana_examples
