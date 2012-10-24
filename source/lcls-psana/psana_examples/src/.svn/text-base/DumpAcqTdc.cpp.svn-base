//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class DumpAcqTdc...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana_examples/DumpAcqTdc.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
#include "psddl_psana/acqiris.ddl.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

using namespace psana_examples;
PSANA_MODULE_FACTORY(DumpAcqTdc)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana_examples {

//----------------
// Constructors --
//----------------
DumpAcqTdc::DumpAcqTdc (const std::string& name)
  : Module(name)
{
  // get the values from configuration or use defaults
  m_src = configStr("source", "DetInfo(:AcqTDC.0)");
}

//--------------
// Destructor --
//--------------
DumpAcqTdc::~DumpAcqTdc ()
{
}

// Method which is called at the beginning of the calibration cycle
void 
DumpAcqTdc::beginCalibCycle(Event& evt, Env& env)
{
  MsgLog(name(), trace, "in beginCalibCycle()");

  shared_ptr<Psana::Acqiris::TdcConfigV1> tdcConfig = env.configStore().get(m_src);
  if (tdcConfig.get()) {

    WithMsgLog(name(), info, str) {
      
      str << "Acqiris::TdcConfigV1:";
      
      const ndarray<Psana::Acqiris::TdcChannel, 1>& channels = tdcConfig->channels();
      for (int ch = 0; ch < Psana::Acqiris::TdcConfigV1::NChannels; ++ ch) {
        const Psana::Acqiris::TdcChannel& chan = channels[ch];
        str << "\n  channel " << ch << ": slope=" << chan.slope() 
            << " mode=" << chan.mode() << " level=" << chan.level();
      }

      const ndarray<Psana::Acqiris::TdcAuxIO, 1>& auxio = tdcConfig->auxio();
      for (int ch = 0; ch < Psana::Acqiris::TdcConfigV1::NAuxIO; ++ ch) {
        const Psana::Acqiris::TdcAuxIO& chan = auxio[ch];
        str << "\n  auxio " << ch << ": channel=" << chan.channel() 
            << " mode=" << chan.mode() << " term=" << chan.term();
      }

      const Psana::Acqiris::TdcVetoIO& veto = tdcConfig->veto();
      str << "\n  veto: channel=" << veto.channel() 
          << " mode=" << veto.mode() << " term=" << veto.term();
      
    }
  }
}

// Method which is called with event data
void 
DumpAcqTdc::event(Event& evt, Env& env)
{
  shared_ptr<Psana::Acqiris::TdcDataV1> tdcData = evt.get(m_src);
  if (tdcData.get()) {
  
    const ndarray<Psana::Acqiris::TdcDataV1_Item, 1>& data = tdcData->data();
    for (unsigned i = 0; i < data.shape()[0]; ++ i) {
      
      const Psana::Acqiris::TdcDataV1_Item& item = data[i];
      
      if (item.source() == Psana::Acqiris::TdcDataV1_Item::Comm) {
        
        const Psana::Acqiris::TdcDataV1Common& comm = 
            static_cast<const Psana::Acqiris::TdcDataV1Common&>(item);
        MsgLog(name(), info, "Acqiris::TdcDataV1: item=" << i 
             << " type=TdcDataV1Common" 
             << " source=" << comm.source()
             << " nhits= " << comm.nhits()
             << " overflow= " << int(comm.overflow()) );
      
      } else if (item.source() == Psana::Acqiris::TdcDataV1_Item::AuxIO) {
      
        const Psana::Acqiris::TdcDataV1Marker& mark = 
            static_cast<const Psana::Acqiris::TdcDataV1Marker&>(item);
        MsgLog(name(), info, "Acqiris::TdcDataV1: item=" << i 
             << " type=TdcDataV1Marker" 
             << " source=" << mark.source()
             << " type= " << mark.type() );

      } else {
        
        const Psana::Acqiris::TdcDataV1Channel& chan = 
            static_cast<const Psana::Acqiris::TdcDataV1Channel&>(item);
        MsgLog(name(), info, "Acqiris::TdcDataV1: item=" << i 
             << " type=TdcDataV1Channel" 
             << " source=" << chan.source()
             << " ticks= " << chan.ticks()
             << " overflow= " << int(chan.overflow())
             << " time= " << chan.time() );

      }
      
    }

  }
}

} // namespace psana_examples
