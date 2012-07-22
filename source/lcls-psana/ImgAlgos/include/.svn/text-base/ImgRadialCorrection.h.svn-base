#ifndef IMGALGOS_IMGRADIALCORRECTION_H
#define IMGALGOS_IMGRADIALCORRECTION_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class ImgRadialCorrection.
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

class ImgRadialCorrection : public Module {
public:

  typedef double amplitude_t; 

  // Default constructor
  ImgRadialCorrection (const std::string& name) ;

  // Destructor
  virtual ~ImgRadialCorrection () ;

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
  void     printInputParameters();
  bool     getAndProcImage(Event& evt);
  bool     procImage(Event& evt);
  void     initPixGeometry();
  unsigned get_img_index(unsigned r, unsigned c);
  unsigned index_for_value(double v, double vmin, double vmax, unsigned nbins);
  void     bookHistograms();
  void     resetHistograms();
  void     accumulateHistograms();
  void     normalizeHistograms();
  void     saveHistograms();
  void     applyRadialCorrection();
  void     saveCorrectedImage();
  void     add_corrected_img_in_event(Event& evt);

  void     printEventId(Event& evt);
  string   stringTimeStamp(Event& evt);
  string   stringEventN();
  //void   savePeaksInEvent(Event& evt);

private:

  enum{ MAX_IMG_SIZE=4000000 };
  enum{ MARGIN=10, MARGIN1=11 };

  //Source      m_source;
  Pds::Src    m_src;
  std::string m_str_src;
  std::string m_inkey;
  std::string m_outkey;
  double      m_xcenter;
  double      m_ycenter;
  double      m_rmin;
  double      m_rmax;
  unsigned    m_rnbins;
  double      m_phimin;
  double      m_phimax;
  unsigned    m_pnbins;
  unsigned    m_rpsize;
  long        m_event;
  unsigned    m_print_bits;
  long        m_count;

  CSPadPixCoords::Image2D<double> *m_img2d;
  ndarray<double,2> *m_ndarr;
  //CSPadPixCoords::Image2D<double> *m_work2d;

  double m_data_arr[MAX_IMG_SIZE];
  //double m_weights[MARGIN][MARGIN];
  //double m_work_arr[MAX_IMG_SIZE];
  TimeInterval *m_time;

  unsigned    m_nrows;
  unsigned    m_ncols;
  double*     m_radval;
  double*     m_phival;
  unsigned*   m_radind;
  unsigned*   m_phiind;
  double*     m_r_amp;
  double*     m_rp_amp;
  unsigned*   m_r_sta;
  unsigned*   m_rp_sta;
};

} // namespace ImgAlgos

#endif // IMGALGOS_IMGRADIALCORRECTION_H
