#ifndef CHUCK_ANA_PKG_CHUCK_ANA_MOD_H
#define CHUCK_ANA_PKG_CHUCK_ANA_MOD_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class chuck_ana_mod.
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

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

namespace chuck_ana_pkg {

/// @addtogroup chuck_ana_pkg

/**
 *  @ingroup chuck_ana_pkg
 *
 *  @brief Example module class for psana
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version \$Id$
 *
 *  @author Chunhong Yoon
 */

class chuck_ana_mod : public Module {
public:

  // Default constructor
  chuck_ana_mod (const std::string& name) ;

  // Destructor
  virtual ~chuck_ana_mod () ;

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

  virtual double getPhotonEnergyeV(Event& evt, Env& env);

  virtual double getPulseEnergymJ(Event& evt, Env& env);

	double cspadSum;
	double cspad2x2Sum;
	double goodPixel;
	double goodPixel2x2;

protected:

private:

  // Data members, this is for example purposes only
  Source m_src;         // Data source set from config file
  //unsigned m_maxEvents;
  //bool m_filter;
  //long m_count;
  bool m_generateDark;
  bool m_outputFluores;
  bool m_generateHistogram;
  bool m_generatePixelFluctuation;
  bool m_generateMeanAsicDark;
  bool m_lookAtCsPad2x1;
  bool m_lookAtGain;
  bool m_lookAtSubregion;
  std::string m_darkFile_388x185;
  std::string m_darkSubRegionFile_55x105;
  std::string m_gainmapFile_388x370;
  std::string m_commonModeFile_1D;

  // Data members
  std::string m_key;
  Source m_srcEvr;
  Source m_srcBeam;
  Source m_srcFee;
  Source m_srcCav;
  Source m_srcAcq;
  Source m_srcCam;
  Source m_src2x2;
};

} // namespace chuck_ana_pkg

#endif // CHUCK_ANA_PKG_CHUCK_ANA_MOD_H
