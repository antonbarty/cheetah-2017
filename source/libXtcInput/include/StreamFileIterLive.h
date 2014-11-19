#ifndef XTCINPUT_STREAMFILEITERLIVE_H
#define XTCINPUT_STREAMFILEITERLIVE_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: StreamFileIterLive.h 6374 2013-05-23 01:00:53Z salnikov@SLAC.STANFORD.EDU $
//
// Description:
//	Class StreamFileIterLive.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <set>
#include <boost/shared_ptr.hpp>

//----------------------
// Base Class Headers --
//----------------------
#include "XtcInput/StreamFileIterI.h"

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "XtcInput/LiveFilesDB.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

namespace XtcInput {

/// @addtogroup XtcInput

/**
 *  @ingroup XtcInput
 *
 *  @brief Implementation of StreamFileIterI interface which works with live data.
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version $Id: StreamFileIterLive.h 6374 2013-05-23 01:00:53Z salnikov@SLAC.STANFORD.EDU $
 *
 *  @author Andy Salnikov
 */

class StreamFileIterLive : public StreamFileIterI {
public:

  /**
   *  @brief Make iterator instance.
   *
   *  @param[in] expNum    Experiment number
   *  @param[in] run       Run number
   *  @param[in] stream    Stream number, or -1 for all stream, -2 for any one stream
   *  @param[in] liveTimeout Timeout in second to wait for live data
   *  @param[in] filesdb   Database connection
   */
  StreamFileIterLive (unsigned expNum, unsigned run, int stream, unsigned liveTimeout,
      const boost::shared_ptr<LiveFilesDB>& filesdb) ;

  /**
   *  @brief Make iterator instance.
   *
   *  @param[in] expNum    Experiment number
   *  @param[in] run       Run number
   *  @param[in] streams   Stream numbers
   *  @param[in] liveTimeout Timeout in second to wait for live data
   *  @param[in] filesdb   Database connection
   */
  StreamFileIterLive (unsigned expNum, unsigned run, std::vector<int> streams, unsigned liveTimeout,
      const boost::shared_ptr<LiveFilesDB>& filesdb) ;

  // Destructor
  virtual ~StreamFileIterLive () ;

  /**
   *  @brief Return chunk iterator for next stream.
   *
   *  Zero pointer is returned after last stream.
   */
  virtual boost::shared_ptr<ChunkFileIterI> next();

  /**
   *  @brief Return stream number for the set of files returned from last next() call.
   */
  virtual unsigned stream() const;

protected:

private:

  typedef std::set<unsigned> Streams;

  unsigned m_expNum;
  unsigned m_run;
  std::vector<int> m_stream;
  unsigned m_liveTimeout;
  boost::shared_ptr<LiveFilesDB> m_filesdb;
  bool m_initialized;
  Streams m_streams;
  unsigned m_lastStream;

};

} // namespace XtcInput

#endif // XTCINPUT_STREAMFILEITERLIVE_H
