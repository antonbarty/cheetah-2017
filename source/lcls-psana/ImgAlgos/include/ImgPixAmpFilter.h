#ifndef IMGALGOS_IMGPIXAMPFILTER_H
#define IMGALGOS_IMGPIXAMPFILTER_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class ImgPixAmpFilter.
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
#include "CSPadPixCoords/Image2D.h"
#include "ImgAlgos/TimeInterval.h"

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

class ImgPixAmpFilter : public Module {
public:

  // Default constructor
  ImgPixAmpFilter (const std::string& name) ;

  // Destructor
  virtual ~ImgPixAmpFilter () ;

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
  void setWindowRange();
  bool getAndProcImage(Event& evt);
  bool procImage(Event& evt);

private:

  Pds::Src    m_actualSrc;
  std::string m_src;
  std::string m_key;
  float    m_threshold;
  float    m_xmin;
  float    m_xmax;
  float    m_ymin;
  float    m_ymax;
  unsigned m_numpixmin;
  bool     m_filter;
  unsigned m_print_bits;
  long     m_count;
  long     m_selected;

  size_t   m_nrows;
  size_t   m_ncols;
  size_t   m_rowmin; 
  size_t   m_rowmax;
  size_t   m_colmin;
  size_t   m_colmax;

  CSPadPixCoords::Image2D<double> *m_img2d;
  TimeInterval *m_time;
};

} // namespace ImgAlgos

#endif // IMGALGOS_IMGPIXAMPFILTER_H
