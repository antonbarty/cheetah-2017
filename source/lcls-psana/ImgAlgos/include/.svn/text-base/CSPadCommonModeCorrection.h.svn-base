#ifndef IMGALGOS_CSPADCOMMONMODECORRECTION_H
#define IMGALGOS_CSPADCOMMONMODECORRECTION_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class CSPadCommonModeCorrection.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------

//----------------------
// Base Class Headers --
//----------------------
#include "psana/Module.h"

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "psddl_psana/cspad.ddl.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

namespace ImgAlgos {

/// @addtogroup ImgAlgos

/**
 *  @ingroup ImgAlgos
 *
 *  @brief Psana module which defines the common mode correction as an average over small ASIC amplitudes 
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @see AdditionalClass
 *
 *  @version \$Id$
 *
 *  @author Mikhail S. Dubrovin
 */

class CSPadCommonModeCorrection : public Module {
public:

  enum { MaxQuads   = Psana::CsPad::MaxQuadsPerSensor };
  enum { MaxSectors = Psana::CsPad::SectorsPerQuad    };
  enum { NumColumns = Psana::CsPad::ColumnsPerASIC    };
  enum { NumRows    = Psana::CsPad::MaxRowsPerASIC*2  };
  enum { SectorSize = NumColumns * NumRows            };
 
  // Default constructor
  CSPadCommonModeCorrection (const std::string& name) ;

  // Destructor
  virtual ~CSPadCommonModeCorrection () ;

  /// Method which is called once at the beginning of the job
  virtual void beginJob(Event& evt, Env& env);
  
  /// Method which is called at the beginning of the run
  virtual void beginRun(Event& evt, Env& env);
  
  /// Method which is called at the beginning of the calibration cycle
  virtual void beginCalibCycle(Event& evt, Env& env);
  
  /// Method which is called with event data, this is the only required 
  /// method, all other methods are optional
  virtual void event(Event& evt, Env& env);
  
  /// Method which is called at the end of the calibration cycle
  virtual void endCalibCycle(Event& evt, Env& env);

  /// Method which is called at the end of the run
  virtual void endRun(Event& evt, Env& env);

  /// Method which is called once at the end of the job
  virtual void endJob(Event& evt, Env& env);

protected:
  /// A part of the event method
  void getAndProcessDataset(Event& evt, Env& env, const std::string& key);
  void processQuad(unsigned qNum, const int16_t* data, int16_t* corrdata, float* common_mode);

private:

  //Source    m_str_src;         // Data source set from config file
  std::string m_str_src;
  Pds::Src    m_src; // source address of the data object

  Pds::Src    m_actualSrc;
  
  std::string m_inkey;
  std::string m_outkey;

  unsigned    m_maxEvents;
  int16_t     m_ampThr;
  bool        m_filter;

  unsigned    m_segMask[MaxQuads];  // segment masks per quadrant
  long        m_count;
  int16_t     m_average;
  //int16_t     m_data_corr[MaxQuads][MaxSectors][NumColumns][NumRows];
};

} // namespace ImgAlgos

#endif // IMGALGOS_CSPADCOMMONMODECORRECTION_H
