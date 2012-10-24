//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class DumpEvr...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana_examples/DumpEvr.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
#include "psddl_psana/evr.ddl.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

using namespace psana_examples;
PSANA_MODULE_FACTORY(DumpEvr)

namespace {

  // Bunch of helper methods to print individual data objects

  void print(std::ostream& str, unsigned i, const Psana::EvrData::PulseConfig& pcfg)
  {
    str << "\n  pulse config #" << i
        << ": pulse=" << pcfg.pulse()
        << " polarity=" << int(pcfg.polarity())
        << " prescale=" << pcfg.prescale()
        << " delay=" << pcfg.delay()
        << " width=" << pcfg.width();
  }

  void print(std::ostream& str, unsigned i, const Psana::EvrData::OutputMap& ocfg)
  {
    str << "\n  output config #" << i
        << ": source=" << ocfg.source()
        << " source_id=" << int(ocfg.source_id())
        << " conn=" << ocfg.conn()
        << " conn_id=" << int(ocfg.conn_id());
  }

  void print(std::ostream& str, unsigned i, const Psana::EvrData::OutputMapV2& ocfg)
  {
    str << "\n  output config #" << i
        << ": source=" << ocfg.source()
        << " source_id=" << int(ocfg.source_id())
        << " conn=" << ocfg.conn()
        << " conn_id=" << int(ocfg.conn_id())
        << " module=" << int(ocfg.module());
  }

  void print(std::ostream& str, unsigned i, const Psana::EvrData::PulseConfigV3& pcfg)
  {
    str << "\n  pulse config #" << i
        << ": pulseId=" << pcfg.pulseId()
        << " polarity=" << int(pcfg.polarity())
        << " prescale=" << pcfg.prescale()
        << " delay=" << pcfg.delay()
        << " width=" << pcfg.width();
  }

  void print(std::ostream& str, unsigned i, const Psana::EvrData::EventCodeV3& ecfg)
  {
    str << "\n  event code #" << i
        << ": code=" << ecfg.code()
        << " isReadout=" << int(ecfg.isReadout())
        << " isTerminator=" << int(ecfg.isTerminator())
        << " maskTrigger=" << ecfg.maskTrigger()
        << " maskSet=" << ecfg.maskSet()
        << " maskClear=" << ecfg.maskClear();

  }

  void print(std::ostream& str, unsigned i, const Psana::EvrData::EventCodeV4& ecfg)
  {
    str << "\n  event code #" << i
        << ": code=" << ecfg.code()
        << " isReadout=" << int(ecfg.isReadout())
        << " isTerminator=" << int(ecfg.isTerminator())
        << " reportDelay=" << ecfg.reportDelay()
        << " reportWidth=" << ecfg.reportWidth()
        << " maskTrigger=" << ecfg.maskTrigger()
        << " maskSet=" << ecfg.maskSet()
        << " maskClear=" << ecfg.maskClear();
  }

  void print(std::ostream& str, unsigned i, const Psana::EvrData::EventCodeV5& ecfg)
  {
    str << "\n  event code #" << i
        << ": code=" << ecfg.code()
        << " isReadout=" << int(ecfg.isReadout())
        << " isCommand=" << int(ecfg.isCommand())
        << " isLatch=" << int(ecfg.isLatch())
        << " reportDelay=" << ecfg.reportDelay()
        << " reportWidth=" << ecfg.reportWidth()
        << " maskTrigger=" << ecfg.maskTrigger()
        << " maskSet=" << ecfg.maskSet()
        << " maskClear=" << ecfg.maskClear();
  }

  void print(std::ostream& str, unsigned i, const Psana::EvrData::SequencerEntry& e)
  {
    str << "\n    entry #" << i <<  " delay=" << e.delay() << " eventcode=" << e.eventcode();
  }

  void print(std::ostream& str, unsigned i, const Psana::EvrData::IOChannel& ioch)
  {
    str << "\n  io channel #" << i
        << ": name=" << ioch.name()
        << " infos=[";
    for (unsigned d = 0; d != ioch.ninfo(); ++ d) {
      str << " " << Pds::DetInfo::name(ioch.infos()[d]);
    }
    str << " ]";
  }

  void print(std::ostream& str, unsigned i, const Psana::EvrData::FIFOEvent& f)
  {
    str << "\n    fifo event #" << i
        <<  " timestampHigh=" << f.timestampHigh()
        <<  " timestampLow=" << f.timestampLow()
        << " eventCode=" << f.eventCode();
  }

  template <typename T>
  void print_array(std::ostream& str, const ndarray<T, 1>& array) {
    for (unsigned i = 0; i < array.size(); ++ i) {
      ::print(str, i, array[i]);
    }
  }

}

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana_examples {

//----------------
// Constructors --
//----------------
DumpEvr::DumpEvr (const std::string& name)
  : Module(name)
{
  m_src = configStr("source", "DetInfo(:Evr)");
}

//--------------
// Destructor --
//--------------
DumpEvr::~DumpEvr ()
{
}

// Method which is called at the beginning of the calibration cycle
void 
DumpEvr::beginCalibCycle(Event& evt, Env& env)
{
  MsgLog(name(), trace, "in beginCalibCycle()");

  // Try to get V1 config object
  shared_ptr<Psana::EvrData::ConfigV1> config1 = env.configStore().get(m_src);
  if (config1.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "EvrData::ConfigV1: npulses = " << config1->npulses()
          << " noutputs = " << config1->noutputs();

      ::print_array(str, config1->pulses());
      ::print_array(str, config1->output_maps());

    }
    
  }

  // Try to get V2 config object
  shared_ptr<Psana::EvrData::ConfigV2> config2 = env.configStore().get(m_src);
  if (config2.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "EvrData::ConfigV2: npulses = " << config2->npulses()
          << " noutputs = " << config2->noutputs()
          << " beam = " << config2->beam()
          << " rate = " << config2->rate() ;

      ::print_array(str, config2->pulses());
      ::print_array(str, config2->output_maps());

    }
    
  }

  // Try to get V3 config object
  shared_ptr<Psana::EvrData::ConfigV3> config3 = env.configStore().get(m_src);
  if (config3.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "EvrData::ConfigV3: npulses = " << config3->npulses()
          << " noutputs = " << config3->noutputs()
          << " neventcodes = " << config3->neventcodes();

      ::print_array(str, config3->pulses());
      ::print_array(str, config3->output_maps());
      ::print_array(str, config3->eventcodes());

    }
    
  }

  // Try to get V4 config object
  shared_ptr<Psana::EvrData::ConfigV4> config4 = env.configStore().get(m_src);
  if (config4.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "EvrData::ConfigV4: npulses = " << config4->npulses()
          << " noutputs = " << config4->noutputs()
          << " neventcodes = " << config4->neventcodes();

      ::print_array(str, config4->pulses());
      ::print_array(str, config4->output_maps());
      ::print_array(str, config4->eventcodes());

    }
    
  }

  // Try to get V5 config object
  shared_ptr<Psana::EvrData::ConfigV5> config5 = env.configStore().get(m_src);
  if (config5.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "EvrData::ConfigV5: npulses = " << config5->npulses()
          << " noutputs = " << config5->noutputs()
          << " neventcodes = " << config5->neventcodes();

      ::print_array(str, config5->pulses());
      ::print_array(str, config5->output_maps());
      ::print_array(str, config5->eventcodes());

      const Psana::EvrData::SequencerConfigV1& scfg = config5->seq_config();
      str << "\n  seq_config: sync_source=" << scfg.sync_source()
          << " beam_source=" << scfg.beam_source()
          << " length=" << scfg.length()
          << " cycles=" << scfg.cycles();

      ::print_array(str, scfg.entries());

    }
    
  }

  // Try to get V6 config object
  shared_ptr<Psana::EvrData::ConfigV6> config6 = env.configStore().get(m_src);
  if (config6.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "EvrData::ConfigV5: npulses = " << config6->npulses()
          << " noutputs = " << config6->noutputs()
          << " neventcodes = " << config6->neventcodes();

      ::print_array(str, config6->pulses());
      ::print_array(str, config6->output_maps());
      ::print_array(str, config6->eventcodes());

      const Psana::EvrData::SequencerConfigV1& scfg = config6->seq_config();
      str << "\n  seq_config: sync_source=" << scfg.sync_source()
          << " beam_source=" << scfg.beam_source()
          << " length=" << scfg.length()
          << " cycles=" << scfg.cycles();

      ::print_array(str, scfg.entries());

    }
    
  }

  shared_ptr<Psana::EvrData::IOConfigV1> iocfg1 = env.configStore().get(m_src);
  if (iocfg1.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "EvrData::IOConfigV1: nchannels = " << iocfg1->nchannels()
          << " conn = " << iocfg1->conn();

      ::print_array(str, iocfg1->channels());

    }
  }
  
}

// Method which is called with event data
void 
DumpEvr::event(Event& evt, Env& env)
{
  shared_ptr<Psana::EvrData::DataV3> data3 = evt.get(m_src);
  if (data3.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "Encoder::DataV3: numFifoEvents=" << data3->numFifoEvents();
      ::print_array(str, data3->fifoEvents());
    }
  }

}
  
} // namespace psana_examples
