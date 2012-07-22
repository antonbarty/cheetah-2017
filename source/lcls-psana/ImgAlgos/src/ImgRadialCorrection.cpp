//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class ImgRadialCorrection...
//
// Author List:
//      Mikhail S. Dubrovin
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "ImgAlgos/ImgRadialCorrection.h"

//-----------------
// C/C++ Headers --
//-----------------
#define _USE_MATH_DEFINES // for M_PI
#include <cmath> // for sqrt, atan2
//#include <math.h> // for exp, M_PI
#include <fstream> // for ofstream

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
#include "psddl_psana/camera.ddl.h"
#include "PSEvt/EventId.h"
//#include "PSTime/Time.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------
#include <iomanip> // for setw, setfill
#include <sstream> // for streamstring
#include <iostream>// for setf

// This declares this class as psana module
using namespace std;
using namespace ImgAlgos;
PSANA_MODULE_FACTORY(ImgRadialCorrection)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace ImgAlgos {

//----------------
// Constructors --
//----------------
ImgRadialCorrection::ImgRadialCorrection (const std::string& name)
  : Module(name)
  , m_str_src()
  , m_inkey()
  , m_outkey()
  , m_xcenter()
  , m_ycenter()
  , m_rmin()
  , m_rmax()
  , m_rnbins()
  , m_phimin()
  , m_phimax()
  , m_pnbins()
  , m_event()   
  , m_print_bits()
  , m_count(0)
{
  m_str_src    = configStr("source",     "DetInfo()");
  m_inkey      = configStr("inkey",               ""); // default means raw data
  m_outkey     = configStr("outkey", "rad_corrected"); // "rc_Image2D" means corrected image as CSPadPixCoords::Image2D<double>
  m_xcenter    = config   ("xcenter",            850);
  m_ycenter    = config   ("ycenter",            850);
  m_rmin       = config   ("rmin",                10);
  m_rmax       = config   ("rmax",              1000);
  m_pnbins     = config   ("n_phi_bins",          12);
  m_event      = config   ("event",                0);
  m_print_bits = config   ("print_bits",           0);

  m_phimin   = -M_PI;
  m_phimax   =  M_PI;
  m_rnbins   =  unsigned(m_rmax - m_rmin); 
  m_rpsize   =  m_pnbins * m_rnbins;

  bookHistograms();
}


//--------------
// Destructor --
//--------------
ImgRadialCorrection::~ImgRadialCorrection ()
{
}

/// Method which is called once at the beginning of the job
void 
ImgRadialCorrection::beginJob(Event& evt, Env& env)
{
  m_time = new TimeInterval();
}

/// Method which is called at the beginning of the run
void 
ImgRadialCorrection::beginRun(Event& evt, Env& env)
{
  if( m_print_bits & 1 ) printInputParameters();
}

/// Method which is called at the beginning of the calibration cycle
void 
ImgRadialCorrection::beginCalibCycle(Event& evt, Env& env)
{
}

/// Method which is called with event data, this is the only required 
/// method, all other methods are optional
void 
ImgRadialCorrection::event(Event& evt, Env& env)
{
  m_time -> startTimeOnce();
  ++ m_count;

  getAndProcImage(evt);
    //if ( getAndProcImage(evt) ) { ++ m_selected; return; } // if event is selected
    //else                        { skip();        return; } // if event is discarded
}
  
/// Method which is called at the end of the calibration cycle
void 
ImgRadialCorrection::endCalibCycle(Event& evt, Env& env)
{
}

/// Method which is called at the end of the run
void 
ImgRadialCorrection::endRun(Event& evt, Env& env)
{
}

/// Method which is called once at the end of the job
void 
ImgRadialCorrection::endJob(Event& evt, Env& env)
{
  if( m_print_bits & 2 ) {
    MsgLog(name(), info, "Total number of events processed: " << m_count);
    m_time -> stopTime(m_count);
  }
}

//--------------------
//--------------------
//--------------------
//--------------------

// Print input parameters
void 
ImgRadialCorrection::printInputParameters()
{
  WithMsgLog(name(), info, log) {
    log << "\n Input parameters:"
        << "\n source     : " << m_str_src
	<< "\n inkey      : " << m_inkey      
	<< "\n outkey     : " << m_outkey      
	<< "\n xcenter    : " << m_xcenter
	<< "\n ycenter    : " << m_ycenter
	<< "\n rmin       : " << m_rmin   
	<< "\n rmax       : " << m_rmax   
	<< "\n event      : " << m_event   
	<< "\n print_bits : " << m_print_bits;
  }
}

//--------------------

bool
ImgRadialCorrection::getAndProcImage(Event& evt)
{
  //MsgLog(name(), info, "::getAndProcImage(...)");

  shared_ptr< CSPadPixCoords::Image2D<double> > img2d = evt.get(m_str_src, m_inkey, &m_src); 
  if (img2d.get()) {
    if( m_print_bits & 8 ) MsgLog(name(), info, "getAndProcImage(...): Get image as Image2D<double>");
    m_img2d = img2d.get();
    const unsigned shape[] = {m_img2d->getNRows(), m_img2d->getNCols()};
    m_ndarr = new ndarray<double,2>(m_img2d->data(), shape);
    return procImage(evt);
  }

  shared_ptr< ndarray<double,2> > img = evt.get(m_str_src, m_inkey, &m_src);
  if (img.get()) {
    if( m_print_bits & 8 ) MsgLog(name(), info, "getAndProcImage(...): Get image as ndarray<double,2>");
    m_img2d = new CSPadPixCoords::Image2D<double>(img->data(), img->shape()[0], img->shape()[1]);
    m_ndarr = img.get();
    return procImage(evt);
  }

  //shared_ptr<Psana::Camera::FrameV1> frmData = evt.get(m_source);
  shared_ptr<Psana::Camera::FrameV1> frmData = evt.get(m_str_src, "", &m_src);
  if (frmData.get()) {

    //unsigned h      = frmData->height();
    //unsigned w      = frmData->width();
    int offset = frmData->offset();

    //m_data = new double [h*w];
    double *p_data = &m_data_arr[0];
    unsigned ind = 0;

      const ndarray<uint8_t, 2>& data8 = frmData->data8();
      if (not data8.empty()) {
        if( m_print_bits & 8 ) MsgLog(name(), info, "getAndProcImage(...): Get image as ndarray<uint8_t,2>");
	const unsigned *shape = data8.shape();
	ndarray<uint8_t, 2>::const_iterator cit;
	for(cit=data8.begin(); cit!=data8.end(); cit++) { p_data[ind++] = double(int(*cit) - offset); }

        m_ndarr = new ndarray<double,2>(p_data, shape);
        m_img2d = new CSPadPixCoords::Image2D<double>(p_data, shape[0], shape[1]);
        return procImage(evt);
      }

      const ndarray<uint16_t, 2>& data16 = frmData->data16();
      if (not data16.empty()) {
        if( m_print_bits & 8 ) MsgLog(name(), info, "getAndProcImage(...): Get image as ndarray<uint16_t,2>");
	const unsigned *shape = data16.shape();
	ndarray<uint16_t, 2>::const_iterator cit;
	// This loop consumes ~5 ms/event for Opal1000 camera with 1024x1024 image size 
	for(cit=data16.begin(); cit!=data16.end(); cit++) { p_data[ind++] = double(*cit) - offset; }

        m_ndarr = new ndarray<double,2>(p_data, shape);
        m_img2d = new CSPadPixCoords::Image2D<double>(p_data, shape[0], shape[1]);
        return procImage(evt);
      }
  }

    return false; // if the image object is not found in evt
}

//--------------------

bool
ImgRadialCorrection::procImage(Event& evt)
{
  if( m_print_bits & 4 )   printEventId(evt);
  if( m_count == 1 )       initPixGeometry();
                           resetHistograms();
                           accumulateHistograms();
                           normalizeHistograms();
  if( m_count == m_event ) saveHistograms();
                           applyRadialCorrection();
  if( m_count == m_event ) saveCorrectedImage();
                           add_corrected_img_in_event(evt);
  return true;
}

//--------------------

//inline
unsigned
ImgRadialCorrection::index_for_value(double v, double vmin, double vmax, unsigned nbins)
{
  if( v <  vmin ) return 0;
  if( v >= vmax ) return nbins-1;
  return unsigned( double(nbins) * (v-vmin) / (vmax-vmin) );
}

//--------------------

inline
unsigned
ImgRadialCorrection::get_img_index(unsigned r, unsigned c)
{
  return r*m_ncols + c;
}

//--------------------

void 
ImgRadialCorrection::bookHistograms()
{
  m_rp_amp = new double  [m_rnbins * m_pnbins];
  m_rp_sta = new unsigned[m_rnbins * m_pnbins];
  m_r_amp  = new double  [m_rnbins];  
  m_r_sta  = new unsigned[m_rnbins];  
}

//--------------------

void 
ImgRadialCorrection::resetHistograms()
{
  for (unsigned ir=0; ir<m_rnbins; ir++) {
      m_r_amp[ir] = 0;  
      m_r_sta[ir] = 0;  
  }
  
  for (unsigned irp=0; irp<m_rpsize; irp++) {
      m_rp_amp[irp]=0;
      m_rp_sta[irp]=0;
  }
}

//--------------------

void 
ImgRadialCorrection::initPixGeometry()
{
  m_nrows = m_ndarr->shape()[0];
  m_ncols = m_ndarr->shape()[1];
  // cout << "Shape:" << m_nrows << " x " << m_ncols << endl; 

  m_radval = new double   [m_nrows * m_ncols];
  m_phival = new double   [m_nrows * m_ncols];
  m_radind = new unsigned [m_nrows * m_ncols];
  m_phiind = new unsigned [m_nrows * m_ncols];

  for(uint16_t r=0; r<m_nrows; r++) {
    double dy = r - m_ycenter;
    for(uint16_t c=0; c<m_ncols; c++) {
      double dx = c - m_xcenter;
      int ind = get_img_index(r,c);
      m_radval[ind] = std::sqrt(dx*dx+dy*dy);
      m_phival[ind] = std::atan2(dy,dx);
      m_radind[ind] = index_for_value(m_radval[ind], m_rmin,   m_rmax,   m_rnbins);
      m_phiind[ind] = index_for_value(m_phival[ind], m_phimin, m_phimax, m_pnbins);

      //cout << m_radind[ind] << " "
      //     << m_phiind[ind] << " : ";
    }
    // cout << endl;
  }
}

//--------------------

void 
ImgRadialCorrection::accumulateHistograms()
{
  ndarray<double,2> &amparr = *m_ndarr;

  for(uint16_t r=0; r<m_nrows; r++){
    for(uint16_t c=0; c<m_ncols; c++){

      double amp  = amparr[r][c];
      int    ind  = get_img_index(r,c);
      int    irad = m_radind[ind];
      int    iphi = m_phiind[ind];

      m_rp_amp[iphi*m_rnbins+irad] += amp;
      m_rp_sta[iphi*m_rnbins+irad] ++;    
      m_r_amp[irad]                += amp;
      m_r_sta[irad]                ++;
    }
  }
}

//--------------------

void 
ImgRadialCorrection::normalizeHistograms()
{
  for (unsigned ir=0; ir<m_rnbins; ir++) {
    m_r_amp[ir] = (m_r_sta[ir] > 0)? m_r_amp[ir] / m_r_sta[ir] : 0;  
  }
  
  for (unsigned irp=0; irp<m_rpsize; irp++) {
    m_rp_amp[irp]= (m_rp_sta[irp] > 0)? m_rp_amp[irp] / m_rp_sta[irp] : 0;
  }

  // Add the m_r_amp histogram as the 1st bin of m_rp_amp[...]
  //unsigned ip=0;
  //for (unsigned ir=0; ir<m_rnbins; ir++) {
  //  m_rp_amp[ip*m_rnbins+ir] = m_r_amp[ir]; 
  //}
}

//--------------------

void 
ImgRadialCorrection::saveHistograms()
{
  //CSPadPixCoords::Image2D<double> *arr2d =  new CSPadPixCoords::Image2D<double>(m_rp_amp, m_pnbins, m_rnbins);
  //arr2d -> saveImageInFile("r-phi-arr2d.txt",3);

  string fname; fname="r-phi-arr2d-ev" + stringEventN() + ".txt";
  ofstream file(fname.c_str(),ios_base::out);

  for (unsigned ir=0; ir<m_rnbins; ir++) {
    for (unsigned ip=0; ip<m_pnbins; ip++) {

      file << m_rp_amp[ip*m_rnbins+ir] << "  "; 
    }
      file << endl;
  }

  file.close();
}

//--------------------

void 
ImgRadialCorrection::applyRadialCorrection()
{
  //double *p_data = &m_data_arr[0];

  ndarray<double,2> &amparr = *m_ndarr;

  for(uint16_t r=0; r<m_nrows; r++){
    for(uint16_t c=0; c<m_ncols; c++){

      double amp  = amparr[r][c];
      int    ind  = get_img_index(r,c);
      int    irad = m_radind[ind];
      int    iphi = m_phiind[ind];
      double rad  = m_radval[ind];
      //double phi  = m_phival[ind];

      if (rad < m_rmin || rad > m_rmax || amp == 0) 
           m_data_arr[ind] = 0;
      else 
	//m_data_arr[ind] = amp - m_r_amp[irad];
          m_data_arr[ind] = amp - m_rp_amp[iphi*m_rnbins+irad];
    }
  }
}

//--------------------

void 
ImgRadialCorrection::saveCorrectedImage()
{
  CSPadPixCoords::Image2D<double> *arr2d =  new CSPadPixCoords::Image2D<double>(m_data_arr, m_nrows, m_ncols);
  string fname; fname="img-r-corrected-ev" + stringEventN() + ".txt";
  arr2d -> saveImageInFile(fname,0);
}

//--------------------

void 
ImgRadialCorrection::printEventId(Event& evt)
{
  shared_ptr<PSEvt::EventId> eventId = evt.get();
  if (eventId.get()) {
    //MsgLog( name(), info, "Event="  << m_count << " ID: " << *eventId);
    MsgLog( name(), info, "Event="  << m_count << " time: " << stringTimeStamp(evt) );
  }
}

///--------------------

std::string
ImgRadialCorrection::stringTimeStamp(Event& evt)
{
  shared_ptr<PSEvt::EventId> eventId = evt.get();
  if (eventId.get()) {
    return (eventId->time()).asStringFormat("%Y%m%dT%H:%M:%S%f"); //("%Y-%m-%d %H:%M:%S%f%z");
  }
  return std::string("Time-stamp-is-unavailable");
}

//--------------------

string 
ImgRadialCorrection::stringEventN()
{
  stringstream ssEvNum; ssEvNum << setw(6) << setfill('0') << m_count;
  return ssEvNum.str();
}

//--------------------

void
ImgRadialCorrection::add_corrected_img_in_event(Event& evt)
{
  if(m_outkey == "rc_Image2D") {

    shared_ptr< CSPadPixCoords::Image2D<double> > img2d( new CSPadPixCoords::Image2D<double>(m_data_arr, m_nrows, m_ncols) );
    evt.put(img2d, m_src, m_outkey);

  } else {

    const unsigned shape[] = {m_nrows, m_ncols};
    shared_ptr< ndarray<double,2> > img2d( new ndarray<double,2>(m_data_arr,shape) );
    evt.put(img2d, m_src, m_outkey);
  }
}

//--------------------
//--------------------

} // namespace ImgAlgos

//---------EOF--------
