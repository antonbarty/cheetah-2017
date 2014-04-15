#ifndef XTCINPUT_RUNFILEITERLIVE_H
#define XTCINPUT_RUNFILEITERLIVE_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: RunFileIterLive.h 6040 2013-03-29 23:32:41Z salnikov@SLAC.STANFORD.EDU $
//
// Description:
//	Class RunFileIterLive.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

//----------------------
// Base Class Headers --
//----------------------
#include "XtcInput/RunFileIterI.h"

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
 *  @brief Implementation of RunFileIterI interface working with live data.
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version $Id: RunFileIterLive.h 6040 2013-03-29 23:32:41Z salnikov@SLAC.STANFORD.EDU $
 *
 *  @author Andy Salnikov
 */

class RunFileIterLive : public RunFileIterI {
public:

  /**
   *  @brief Make iterator instance.
   *
   *  Constructor takes sequence of run numbers in the form of iterators.
   *
   *  @param[in] begin     Iterator pointing to the beginning of run number sequence
   *  @param[in] end       Iterator pointing to the end of run number sequence
   *  @param[in] expNum    Experiment number
   *  @param[in] streams   Stream numbers or -1 for all stream, -2 for any one stream
   *  @param[in] liveTimeout Specifies timeout in second when reading live data
   *  @param[in] dbConnStr Database connection string
   *  @param[in] table     Database table name
   *  @param[in] dir       Directory to look for live files
   */
  template <typename Iter>
    RunFileIterLive (Iter begin, Iter end, unsigned expNum, std::vector<int> streams, unsigned liveTimeout,
      const std::string& dbConnStr, const std::string& table, const std::string& dir)
    : RunFileIterI()
    , m_runs(begin, end)
    , m_expNum(expNum)
    , m_stream(streams)
    , m_liveTimeout(liveTimeout)
    , m_run(0)
    , m_filesdb(boost::make_shared<LiveFilesDB>(dbConnStr, table, dir))
  {
  }

  /**
   *  @brief Make iterator instance.
   *
   *  Constructor takes sequence of run numbers in the form of iterators.
   *
   *  @param[in] begin     Iterator pointing to the beginning of run number sequence
   *  @param[in] end       Iterator pointing to the end of run number sequence
   *  @param[in] expNum    Experiment number
   *  @param[in] stream    Stream number, or -1 for all stream, -2 for any one stream
   *  @param[in] liveTimeout Specifies timeout in second when reading live data
   *  @param[in] dbConnStr Database connection string
   *  @param[in] table     Database table name
   *  @param[in] dir       Directory to look for live files
   */
  template <typename Iter>
  RunFileIterLive (Iter begin, Iter end, unsigned expNum, int stream, unsigned liveTimeout,
      const std::string& dbConnStr, const std::string& table, const std::string& dir)
    : RunFileIterI()
    , m_runs(begin, end)
    , m_expNum(expNum)
    , m_stream(stream)
    , m_liveTimeout(liveTimeout)
    , m_run(0)
    , m_filesdb(boost::make_shared<LiveFilesDB>(dbConnStr, table, dir))
  {
    m_stream.push_back(stream);
  }

  // Destructor
  virtual ~RunFileIterLive () ;

  /**
   *  @brief Return stream iterator for next run.
   *
   *  Zero pointer is returned after last run.
   */
  virtual boost::shared_ptr<StreamFileIterI> next();

  /**
   *  @brief Return run number for the set of files returned from last next() call.
   */
  virtual unsigned run() const;

protected:

private:

  typedef std::set<unsigned> Runs;
  
  Runs m_runs;
  unsigned m_expNum;
  std::vector<int> m_stream;
  unsigned m_liveTimeout;
  unsigned m_run;
  boost::shared_ptr<LiveFilesDB> m_filesdb;

};

} // namespace XtcInput

#endif // XTCINPUT_RUNFILEITERLIVE_H
