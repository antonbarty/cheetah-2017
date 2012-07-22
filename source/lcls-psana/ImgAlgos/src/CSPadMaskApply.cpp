//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class CSPadMaskApply...
//
// Author List:
//      Mikhail S. Dubrovin
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "ImgAlgos/CSPadMaskApply.h"

//-----------------
// C/C++ Headers --
//-----------------
//#include <time.h>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
// to work with detector data include corresponding 
// header from psddl_psana package
#include "PSEvt/EventId.h"
#include "cspad_mod/DataT.h"
#include "cspad_mod/ElementT.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

// This declares this class as psana module
using namespace Psana;
using namespace ImgAlgos;
PSANA_MODULE_FACTORY(CSPadMaskApply)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace ImgAlgos {

//----------------
// Constructors --
//----------------
CSPadMaskApply::CSPadMaskApply (const std::string& name)
  : Module(name)
  , m_str_src()
  , m_inkey()
  , m_outkey()
  , m_fname()
  , m_masked_amp()
  , m_print_bits()
  , m_count(0)
{
  // get the values from configuration or use defaults
  m_str_src     = configStr("source",     "DetInfo(:Cspad)");
  m_inkey       = configStr("inkey",      "");
  m_outkey      = configStr("outkey",     "mask_applyed");
  m_fname       = configStr("mask_fname", "cspad_mask.dat");
  m_masked_amp  = config   ("masked_amp", 0);
  m_print_bits  = config   ("print_bits", 0);

  std::fill_n(&m_common_mode[0], int(MaxSectors), float(0));
}

//--------------
// Destructor --
//--------------
CSPadMaskApply::~CSPadMaskApply ()
{
}

/// Method which is called once at the beginning of the job
void 
CSPadMaskApply::beginJob(Event& evt, Env& env)
{
  getMaskArray();
  if( m_print_bits & 1 ) printInputParameters();
  if( m_print_bits & 8 ) printMaskArray();
}

/// Method which is called at the beginning of the run
void 
CSPadMaskApply::beginRun(Event& evt, Env& env)
{
  // Find all configuration objects matching the source address
  // provided in configuration. If there is more than one configuration 
  // object is found then complain and stop.
  
  std::string src = configStr("source", "DetInfo(:Cspad)");
  int count = 0;
  
  // need to know segment mask which is availabale in configuration only
  shared_ptr<Psana::CsPad::ConfigV1> config1 = env.configStore().get(m_str_src, &m_src);
  if (config1.get()) {
    for (int i = 0; i < MaxQuads; ++i) { m_segMask[i] = config1->asicMask()==1 ? 0x3 : 0xff; }
    ++ count;
  }
  
  shared_ptr<Psana::CsPad::ConfigV2> config2 = env.configStore().get(m_str_src, &m_src);
  if (config2.get()) {
    for (int i = 0; i < MaxQuads; ++i) { m_segMask[i] = config2->roiMask(i); }
    ++ count;
  }

  shared_ptr<Psana::CsPad::ConfigV3> config3 = env.configStore().get(m_str_src, &m_src);
  if (config3.get()) {
    for (int i = 0; i < MaxQuads; ++i) { m_segMask[i] = config3->roiMask(i); }
    ++ count;
  }

  if (not count) {
    MsgLog(name(), error, "No CSPad configuration objects found. Terminating.");
    terminate();
    return;
  }
  
  if (count > 1) {
    MsgLog(name(), error, "Multiple CSPad configuration objects found, use more specific source address. Terminating.");
    terminate();
    return;
  }

  MsgLog(name(), info, "Found CSPad object with address " << m_src);
  if (m_src.level() != Pds::Level::Source) {
    MsgLog(name(), error, "Found CSPad configuration object with address not at Source level. Terminating.");
    terminate();
    return;
  }

  const Pds::DetInfo& dinfo = static_cast<const Pds::DetInfo&>(m_src);
  // validate that this is indeed CSPad, should always be true, but
  // additional protection here should not hurt
  if (dinfo.device() != Pds::DetInfo::Cspad) {
    MsgLog(name(), error, "Found CSPad configuration object with invalid address. Terminating.");
    terminate();
    return;
  }
}

/// Method which is called at the beginning of the calibration cycle
void 
CSPadMaskApply::beginCalibCycle(Event& evt, Env& env)
{
}

/// Method which is called with event data, this is the only required 
/// method, all other methods are optional
void 
CSPadMaskApply::event(Event& evt, Env& env)
{
  applyMask(evt);
}

/// Method which is called at the end of the calibration cycle
void 
CSPadMaskApply::endCalibCycle(Event& evt, Env& env)
{
}

/// Method which is called at the end of the run
void 
CSPadMaskApply::endRun(Event& evt, Env& env)
{
}

/// Method which is called once at the end of the job
void 
CSPadMaskApply::endJob(Event& evt, Env& env)
{
}

//--------------------

// Print input parameters
void 
CSPadMaskApply::printInputParameters()
{
  WithMsgLog(name(), info, log) {
    log << "\n Input parameters:"
        << "\n source     : " << m_str_src
        << "\n inkey      : " << m_inkey      
        << "\n outkey     : " << m_outkey      
        << "\n fname      : " << m_fname    
        << "\n masked_amp : " << m_masked_amp
        << "\n print_bits : " << m_print_bits
        << "\n";     

    log << "\n MaxQuads   : " << MaxQuads    
        << "\n MaxSectors : " << MaxSectors  
        << "\n NumColumns : " << NumColumns  
        << "\n NumRows    : " << NumRows     
        << "\n SectorSize : " << SectorSize  
        << "\n";
  }
}

//--------------------

void 
CSPadMaskApply::printEventId(Event& evt)
{
  shared_ptr<PSEvt::EventId> eventId = evt.get();
  if (eventId.get()) {
    //MsgLog( name(), info, "Event="  << m_count << " ID: " << *eventId);
    MsgLog( name(), info, "Event="  << m_count << " time: " << stringTimeStamp(evt) );
  }
}

//--------------------

std::string
CSPadMaskApply::stringTimeStamp(Event& evt)
{
  shared_ptr<PSEvt::EventId> eventId = evt.get();
  if (eventId.get()) {
    return (eventId->time()).asStringFormat("%Y%m%dT%H:%M:%S%f"); //("%Y-%m-%d %H:%M:%S%f%z");
  }
  return std::string("Time-stamp-is-unavailable");
}

//--------------------

void
CSPadMaskApply::getMaskArray()
{
  m_mask = new ImgAlgos::CSPadMaskV1(m_fname);
}

//--------------------

void 
CSPadMaskApply::printMaskArray()
{
  m_mask -> print(); 
}

//--------------------

/// Apply the mask and save array in the event
void 
CSPadMaskApply::applyMask(Event& evt)
{
  shared_ptr<Psana::CsPad::DataV1> data1 = evt.get(m_str_src, m_inkey, &m_src);
  if (data1.get()) {

    ++ m_count;

    shared_ptr<cspad_mod::DataV1> newobj(new cspad_mod::DataV1());

    int nQuads = data1->quads_shape()[0];
    for (int iq = 0; iq != nQuads; ++ iq) {

      const CsPad::ElementV1& quad = data1->quads(iq); // get quad object 
      const ndarray<int16_t,3>& data = quad.data();    // get data for quad

      int16_t* corrdata = new int16_t[data.size()];  // allocate memory for corrected quad-array 
      //int16_t* corrdata = &m_corrdata[iq][0][0][0];        // allocate memory for corrected quad-array 
      processQuad(quad.quad(), data.data(), corrdata); // process event for quad

      newobj->append(new cspad_mod::ElementV1(quad, corrdata, m_common_mode));
    }    
    evt.put<Psana::CsPad::DataV1>(newobj, m_src, m_outkey); // put newobj in event 
  }
  
  shared_ptr<Psana::CsPad::DataV2> data2 = evt.get(m_str_src, m_inkey, &m_src);
  if (data2.get()) {

    ++ m_count;

    shared_ptr<cspad_mod::DataV2> newobj(new cspad_mod::DataV2());
    
    int nQuads = data2->quads_shape()[0];
    for (int iq = 0; iq != nQuads; ++ iq) {
      
      const CsPad::ElementV2& quad = data2->quads(iq); // get quad object 
      const ndarray<int16_t,3>& data = quad.data();    // get data for quad

      int16_t* corrdata = new int16_t[data.size()];  // allocate memory for corrected quad-array 
      //int16_t* corrdata = &m_corrdata[iq][0][0][0];        // allocate memory for corrected quad-array 
      processQuad(quad.quad(), data.data(), corrdata); // process event for quad

      newobj->append(new cspad_mod::ElementV2(quad, corrdata, m_common_mode)); 
    } 
    evt.put<Psana::CsPad::DataV2>(newobj, m_src, m_outkey); // put newobj in event 
  }

  if( m_print_bits & 2 ) printEventId(evt);
}

//--------------------

/// Process data for all sectors in quad
void 
CSPadMaskApply::processQuad(unsigned quad, const int16_t* data, int16_t* corrdata)
{
  //cout << "processQuad =" << quad << endl;

  int ind_in_arr = 0;
  for (int sect = 0; sect < MaxSectors; ++ sect) {
    if (m_segMask[quad] & (1 << sect)) {
     
      // beginning of the segment data
      const int16_t* sectData = data     + ind_in_arr*SectorSize;
      int16_t*       corrData = corrdata + ind_in_arr*SectorSize;
      uint16_t*      sectMask = m_mask->getMask(quad,sect);

      // Apply mask
      for (int i = 0; i < SectorSize; ++ i) {

        corrData[i] = (sectMask[i] != 0) ? sectData[i] : m_masked_amp;
      }                
      ++ind_in_arr;
    }
  }  
}

//--------------------

} // namespace ImgAlgos
