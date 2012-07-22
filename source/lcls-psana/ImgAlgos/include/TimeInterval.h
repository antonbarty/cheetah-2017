#ifndef IMGALGOS_TIMEINTERVAL_H
#define IMGALGOS_TIMEINTERVAL_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class TimeInterval.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------

#include <time.h>

//----------------------
// Base Class Headers --
//----------------------


//-------------------------------
// Collaborating Class Headers --
//-------------------------------

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
 *  @brief C++ source file code template.
 *
 *  The first sentence is a brief summary of what the class is for. It is 
 *  followed by more detailed information about how to use the class. 
 *  This doc comment must immediately precede the class definition.
 *
 *  Additional paragraphs with more details may follow; separate paragraphs
 *  with a blank line. The last paragraph before the tags (preceded by @) 
 *  should be the identification and copyright, as below.
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version $Id$
 *
 *  @author Mikhail S. Dubrovin
 */

class TimeInterval  {
public:

  // Default constructor
  TimeInterval () ;

  // Destructor
  virtual ~TimeInterval () ;

  void startTimeOnce();
  void startTime();
  void stopTime(long nevents=0);

protected:

private:

  // Copy constructor and assignment are disabled by default
  TimeInterval ( const TimeInterval& ) ;
  TimeInterval& operator = ( const TimeInterval& ) ;

  int m_status;
  struct timespec m_start, m_stop;

  unsigned m_entrance_counter;

};

} // namespace ImgAlgos

#endif // IMGALGOS_TIMEINTERVAL_H
