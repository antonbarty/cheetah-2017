//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class CSPadArrAverage...
//
// Author List:
//      Mikhail S. Dubrovin
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "ImgAlgos/CSPadArrAverage.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <fstream>
#include <cmath>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
// to work with detector data include corresponding 
// header from psddl_psana package
//#include "psddl_psana/acqiris.ddl.h"

#include "PSEvt/EventId.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

// This declares this class as psana module
using namespace Psana;
using namespace ImgAlgos;

PSANA_MODULE_FACTORY(CSPadArrAverage)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace ImgAlgos {

//----------------
// Constructors --
//----------------
CSPadArrAverage::CSPadArrAverage (const std::string& name)
  : Module(name)
  , m_str_src()
  , m_key()
  , m_aveFile()
  , m_rmsFile()
  , m_print_bits()
  , m_count(0)
  , m_nev_stage1()
  , m_nev_stage2()
  , m_gate_width1()
  , m_gate_width2()
{
  // get the values from configuration or use defaults
  m_str_src = configStr("source",  "DetInfo(:Cspad)");
  m_key     = configStr("key",     "");                 //"calibrated"
  m_aveFile = configStr("avefile", "cspad-ave.dat");
  m_rmsFile = configStr("rmsfile", "cspad-rms.dat");
  m_nev_stage1  = config("evts_stage1", 1<<31U);
  m_nev_stage2  = config("evts_stage2",    100);
  m_gate_width1 = config("gate_width1",      0); 
  m_gate_width2 = config("gate_width2",      0); 
  m_print_bits  = config("print_bits",       0);

  // initialize arrays
  std::fill_n(&m_segMask[0], int(MaxQuads), 0U);

  m_gate_width = 0;
  // resetStatArrays(); // <=== moved to procEvent
}

//--------------
// Destructor --
//--------------
CSPadArrAverage::~CSPadArrAverage ()
{
}

/// Method which is called once at the beginning of the job
void 
CSPadArrAverage::beginJob(Event& evt, Env& env)
{
  if( m_print_bits & 1<<0 ) printInputParameters();
}

/// Method which is called at the beginning of the run
void 
CSPadArrAverage::beginRun(Event& evt, Env& env)
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
CSPadArrAverage::beginCalibCycle(Event& evt, Env& env)
{
}

/// Method which is called with event data, this is the only required 
/// method, all other methods are optional
void 
CSPadArrAverage::event(Event& evt, Env& env)
{
  shared_ptr<Psana::CsPad::DataV1> data1 = evt.get(m_str_src, m_key, &m_src);
  if (data1.get()) {

    ++ m_count;
    setCollectionMode();
    
    int nQuads = data1->quads_shape()[0];
    for (int iq = 0; iq != nQuads; ++ iq) {

      const CsPad::ElementV1& quad = data1->quads(iq);
      const ndarray<int16_t, 3>& data = quad.data();
      collectStat(quad.quad(), data.data());
    }    
  }
  
  shared_ptr<Psana::CsPad::DataV2> data2 = evt.get(m_str_src, m_key, &m_src);
  if (data2.get()) {

    ++ m_count;
    setCollectionMode();
    
    int nQuads = data2->quads_shape()[0];
    for (int iq = 0; iq != nQuads; ++ iq) {
      
      const CsPad::ElementV2& quad = data2->quads(iq);
      const ndarray<int16_t, 3>& data = quad.data();
      collectStat(quad.quad(), data.data());
    } 
  }

   if( m_print_bits & 1<<4 ) printEventId(evt);
}
  
/// Method which is called at the end of the calibration cycle
void 
CSPadArrAverage::endCalibCycle(Event& evt, Env& env)
{
}

/// Method which is called at the end of the run
void 
CSPadArrAverage::endRun(Event& evt, Env& env)
{
}

/// Method which is called once at the end of the job
void 
CSPadArrAverage::endJob(Event& evt, Env& env)
{

  procStatArrays();
  saveCSPadArrayInFile( m_aveFile, m_ave ); // &m_ave[0][0][0][0] );
  saveCSPadArrayInFile( m_rmsFile, m_rms ); // &m_rms[0][0][0][0] );

}

//--------------------

/// Process accumulated stat arrays and evaluate m_ave(rage) and m_rms arrays
void 
CSPadArrAverage::procStatArrays()
{
  if( m_print_bits & 1<<2 ) MsgLog(name(), info, "Process statistics for collected total " << m_count << " events");
  
    for (int iq = 0; iq != MaxQuads; ++ iq) {
      for (int is = 0; is != MaxSectors; ++ is) {
        for (int ic = 0; ic != NumColumns; ++ ic) {
          for (int ir = 0; ir != NumRows; ++ ir) {

	    double stat  = m_stat[iq][is][ic][ir];
            if (stat > 1) {
              double ave   = m_sum[iq][is][ic][ir] / stat;
	      m_ave[iq][is][ic][ir] = ave;
              m_rms[iq][is][ic][ir] = std::sqrt(m_sum2[iq][is][ic][ir] / stat - ave*ave);
            } 
            else 
            {
	      m_ave[iq][is][ic][ir] = 0;
	      m_rms[iq][is][ic][ir] = 0;
            }
          }
        }
      }
    }
}

//--------------------

/// Save 4-d array of CSPad structure in file
void 
CSPadArrAverage::saveCSPadArrayInFile(std::string& fname, double arr[MaxQuads][MaxSectors][NumColumns][NumRows])
{  
  if (not fname.empty()) {
    if( m_print_bits & 1<<3 ) MsgLog(name(), info, "Save CSPad-shaped array in file " << fname.c_str());
    std::ofstream out(fname.c_str());
    for (int iq = 0; iq != MaxQuads; ++ iq) {
      for (int is = 0; is != MaxSectors; ++ is) {
        for (int ic = 0; ic != NumColumns; ++ ic) {
          for (int ir = 0; ir != NumRows; ++ ir) {

            out << arr[iq][is][ic][ir] << ' ';
          }
          out << '\n';
        }
      }
    }
    out.close();
  }
}

//--------------------

/// Reset arrays for statistics accumulation
void
CSPadArrAverage::resetStatArrays()
{
  std::fill_n(&m_stat[0][0][0][0], MaxQuads*MaxSectors*NumColumns*NumRows, 0 );
  std::fill_n(&m_sum [0][0][0][0], MaxQuads*MaxSectors*NumColumns*NumRows, 0.);
  std::fill_n(&m_sum2[0][0][0][0], MaxQuads*MaxSectors*NumColumns*NumRows, 0.);
}

//--------------------

/// Check the event counter and deside what to do next accumulate/change mode/etc.
void 
CSPadArrAverage::setCollectionMode()
{
  // Set the statistics collection mode without gate
  if (m_count == 1 ) {
    m_gate_width = 0;
    resetStatArrays();
    if( m_print_bits & 1<<1 ) MsgLog(name(), info, "Stage 0: Event = " << m_count << " Begin to collect statistics without gate.");
  }

  // Change the statistics collection mode for gated stage 1
  else if (m_count == m_nev_stage1 ) {
    procStatArrays();
    resetStatArrays();
    m_gate_width = m_gate_width1;
    if( m_print_bits & 1<<1 ) MsgLog(name(), info, "Stage 1: Event = " << m_count << " Begin to collect statistics with gate =" << m_gate_width);
  } 

  // Change the statistics collection mode for gated stage 2
  else if (m_count == m_nev_stage1 + m_nev_stage2 ) {
    procStatArrays();
    resetStatArrays();
    m_gate_width = m_gate_width2;
    if( m_print_bits & 1<<1 ) MsgLog(name(), info, "Stage 2: Event = " << m_count << " Begin to collect statistics with gate =" << m_gate_width);
  }
}

//--------------------

/// Collect statistics
void 
CSPadArrAverage::collectStat(unsigned quad, const int16_t* data)
{
  //cout << "collectStat for quad =" << quad << endl;

  int ind_in_arr = 0;
  for (int sect = 0; sect < MaxSectors; ++ sect) {
    if (m_segMask[quad] & (1 << sect)) {
     
      // beginning of the segment data
      unsigned* stat = &m_stat[quad][sect][0][0];
      double*   sum  = &m_sum [quad][sect][0][0];
      double*   sum2 = &m_sum2[quad][sect][0][0];
      double*   ave  = &m_ave [quad][sect][0][0];
      //double*   rms  = &m_rms [quad][sect][0][0];
      const int16_t* segData = data + ind_in_arr*SectorSize;

      // sum
      for (int i = 0; i < SectorSize; ++ i) {

	double amp = double(segData[i]);
	if ( m_gate_width > 0 && abs(amp-ave[i]) > m_gate_width ) continue; // gate_width -> n_rms_gate_width * rms[i]

        stat[i] ++;
        sum [i] += amp;
        sum2[i] += amp*amp;
      }          
      
      ++ind_in_arr;
    }
  }
}

//--------------------

// Print input parameters
void 
CSPadArrAverage::printInputParameters()
{
  WithMsgLog(name(), info, log) {
    log << "\n Input parameters:"
        << "\n source     : " << m_str_src
        << "\n key        : " << m_key      
        << "\n m_aveFile  : " << m_aveFile    
        << "\n m_rmsFile  : " << m_rmsFile    
        << "\n print_bits : " << m_print_bits
        << "\n evts_stage1: " << m_nev_stage1   
        << "\n evts_stage2: " << m_nev_stage2  
        << "\n gate_width1: " << m_gate_width1 
        << "\n gate_width2: " << m_gate_width2 
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
CSPadArrAverage::printEventId(Event& evt)
{
  shared_ptr<PSEvt::EventId> eventId = evt.get();
  if (eventId.get()) {
    MsgLog( name(), info, "Event="  << m_count << " ID: " << *eventId);
  }
}

//--------------------

} // namespace ImgAlgos
