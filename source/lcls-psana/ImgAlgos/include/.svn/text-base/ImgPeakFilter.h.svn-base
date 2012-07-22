#ifndef IMGALGOS_IMGPEAKFILTER_H
#define IMGALGOS_IMGPEAKFILTER_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class ImgPeakFilter.
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
#include "ImgAlgos/ImgPeakFinder.h"

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

class ImgPeakFilter : public Module {
public:

  // Default constructor
  ImgPeakFilter (const std::string& name) ;

  // Destructor
  virtual ~ImgPeakFilter () ;

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
  void printPeaks();
  void printEventId(Event& evt);
  void printInputParameters();
  bool eventIsSelected(Event& evt);
  bool peakSelector();

private:

  // Data members, this is for example purposes only
  
  Pds::Src    m_actualSrc;
  //Source      m_src;
  std::string m_src;
  std::string m_key;
  bool        m_filterIsOn;
  double      m_thr_peak;
  double      m_thr_total;
  unsigned    m_thr_npeaks;  
  unsigned    m_print_bits;
  long        m_count;
  long        m_selected;

  vector<Peak>* m_peaks;
};

} // namespace ImgAlgos

#endif // IMGALGOS_IMGPEAKFILTER_H
