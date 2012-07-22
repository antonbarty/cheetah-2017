#ifndef IMGALGOS_CSPADBKGDSUBTRACT_H
#define IMGALGOS_CSPADBKGDSUBTRACT_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class CSPadBkgdSubtract.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
//#include <iostream>
//#include <string>
//#include <vector>
//#include <fstream>  // open, close etc.

//----------------------
// Base Class Headers --
//----------------------
#include "psana/Module.h"

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "psddl_psana/cspad.ddl.h" // for Psana::CsPad::MaxQuadsPerSensor etc.
#include "ImgAlgos/CSPadBackgroundV1.h"

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
 *  @brief Example module class for psana
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version \$Id$
 *
 *  @author Mikhail S. Dubrovin
 */

class CSPadBkgdSubtract : public Module {
public:

    enum { MaxQuads   = Psana::CsPad::MaxQuadsPerSensor }; // 4
    enum { MaxSectors = Psana::CsPad::SectorsPerQuad    }; // 8
    enum { NumColumns = Psana::CsPad::ColumnsPerASIC    }; // 185 THERE IS A MESS IN ONLINE COLS<->ROWS
    enum { NumRows    = Psana::CsPad::MaxRowsPerASIC*2  }; // 388 THERE IS A MESS IN ONLINE COLS<->ROWS 
    enum { SectorSize = NumColumns * NumRows            }; // 185 * 388
  
   // Default constructor
  CSPadBkgdSubtract (const std::string& name) ;

  // Destructor
  virtual ~CSPadBkgdSubtract () ;

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
  void printInputParameters();
  void printEventId(Event& evt);
  std::string stringTimeStamp(Event& evt);
  void printBkgdArray();
  void getBkgdArray();
  void normalizeBkgd(Event& evt);
  void subtractBkgd(Event& evt);
  void processQuad(unsigned quad, const int16_t* data, int16_t* corrdata);
  double normQuadBkgd(unsigned quad, const int16_t* data);

private:

  // Data members, this is for example purposes only
  
  //Source m_src;         // Data source set from config file

  Pds::Src       m_src;             // source address of the data object
  std::string    m_str_src;         // string with source name
  std::string    m_inkey;
  std::string    m_outkey;
  std::string    m_fname;
  unsigned       m_norm_sector;
  unsigned       m_print_bits;   
 
  long m_count;

  unsigned       m_segMask[MaxQuads];  // segment masks per quadrant
  float          m_common_mode[MaxSectors];
  //double         m_bkgd[MaxQuads][MaxSectors][NumColumns][NumRows];  // averaged per pixel background 
  //int16_t        m_corrdata[MaxQuads][MaxSectors][NumColumns][NumRows];  // max size array for corrected quad data

  double         m_norm_factor;

  ImgAlgos::CSPadBackgroundV1 *m_bkgd;

};

} // namespace ImgAlgos

#endif // IMGALGOS_CSPADBKGDSUBTRACT_H
