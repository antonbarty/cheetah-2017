#ifndef IMGALGOS_TIMESTAMPFILTER_H
#define IMGALGOS_TIMESTAMPFILTER_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class TimeStampFilter.
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
#include "PSTime/Time.h"

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

class TimeStampFilter : public Module {
public:

  // Default constructor
  TimeStampFilter (const std::string& name) ;

  // Destructor
  virtual ~TimeStampFilter () ;

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
  void setInputParameters();
  void printInputParameters();
  void printEventId(Event& evt);
  void getTimeStamp(Event& evt);
  void printTimeStamp();
  bool isSelected();

private:

  // Data members

  std::string m_def_tstamp_min;
  std::string m_def_tstamp_max;
  std::string m_def_tsinterval;

  std::string m_str_tstamp_min;
  std::string m_str_tstamp_max;
  std::string m_str_tsinterval;

  PSTime::Time *m_tmin;
  PSTime::Time *m_tmax;
  PSTime::Time  m_time;
  
  unsigned m_run;  

  bool m_filterIsOn;
  unsigned m_print_bits;
  long m_count;
  long m_selected;
};

} // namespace ImgAlgos

#endif // IMGALGOS_TIMESTAMPFILTER_H
