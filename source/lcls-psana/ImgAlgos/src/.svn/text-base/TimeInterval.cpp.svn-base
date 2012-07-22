//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class TimeInterval...
//
// Author List:
//      Mikhail S. Dubrovin
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "ImgAlgos/TimeInterval.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace ImgAlgos {

//----------------
// Constructors --
//----------------
TimeInterval::TimeInterval()
{
  m_entrance_counter = 0;
  //startTimeOnce();
}

//--------------
// Destructor --
//--------------
TimeInterval::~TimeInterval()
{
}

//--------------------

/// Store and prints time at start of the measured interval
void 
TimeInterval::startTimeOnce()
{
  if( m_entrance_counter > 0 ) return;
      m_entrance_counter ++;

  startTime();
}

//--------------------

/// Store and prints time at start of the measured interval
void 
TimeInterval::startTime()
{
  m_status = clock_gettime( CLOCK_REALTIME, &m_start ); // Get LOCAL time
  struct tm * timeinfo; timeinfo = localtime ( &m_start.tv_sec ); 
  char c_time_buf[80]; strftime (c_time_buf, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
  MsgLog("TimeInterval::startTime", info, "Start time: " << c_time_buf << " and " << m_start.tv_nsec << " nsec");
}

//--------------------

/// Stop and prints time interval since start
void 
TimeInterval::stopTime(long nevents)
{
  m_status = clock_gettime( CLOCK_REALTIME, &m_stop ); // Get LOCAL time
  double dt = m_stop.tv_sec - m_start.tv_sec + 1e-9*(m_stop.tv_nsec - m_start.tv_nsec);
  MsgLog("TimeInterval::stopTime", info, "Time to process "<< nevents << " events is " << dt << " sec, or " << dt/nevents << " sec/event");
}

//--------------------

} // namespace ImgAlgos
