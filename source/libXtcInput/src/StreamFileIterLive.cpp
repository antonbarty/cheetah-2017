//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: StreamFileIterLive.cpp 6040 2013-03-29 23:32:41Z salnikov@SLAC.STANFORD.EDU $
//
// Description:
//	Class StreamFileIterLive...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "XtcInput/StreamFileIterLive.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <unistd.h>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
#include "XtcInput/ChunkFileIterLive.h"
#include "XtcInput/Exceptions.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

namespace {

  const char* logger = "XtcInput.StreamFileIterLive";

}

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace XtcInput {

//----------------
// Constructors --
//----------------
	StreamFileIterLive::StreamFileIterLive (unsigned expNum, unsigned run, std::vector<int> streams, 
    unsigned liveTimeout, const boost::shared_ptr<LiveFilesDB>& filesdb)
  : StreamFileIterI()
  , m_expNum(expNum)
  , m_run(run)
  , m_stream(streams)
  , m_liveTimeout(liveTimeout)
  , m_filesdb(filesdb)
  , m_initialized(false)
  , m_lastStream(0)
{
}

StreamFileIterLive::StreamFileIterLive (unsigned expNum, unsigned run, int stream, 
    unsigned liveTimeout, const boost::shared_ptr<LiveFilesDB>& filesdb)
  : StreamFileIterI()
  , m_expNum(expNum)
  , m_run(run)
  , m_liveTimeout(liveTimeout)
  , m_filesdb(filesdb)
  , m_initialized(false)
  , m_lastStream(0)
{
	m_stream.push_back(stream);
}

//--------------
// Destructor --
//--------------
StreamFileIterLive::~StreamFileIterLive ()
{
}

/**
 *  @brief Return chunk iterator for next stream.
 *
 *  Zero pointer is returned after last stream.
 */
boost::shared_ptr<ChunkFileIterI>
StreamFileIterLive::next()
{
  boost::shared_ptr<ChunkFileIterI> next;

  if (not m_initialized) {

    // first time around get the list of streams from database

    m_initialized = true;

    std::vector<XtcFileName> files = m_filesdb->files(m_expNum, m_run);
    if (files.empty()) {
      throw ErrDbRunLiveData(ERR_LOC, m_run);
    }

    if (not files.empty()) {

      // wait for some time until at least one file appears on disk
      std::time_t t0 = std::time(0);
      bool found = false;
      while (not found) {
        for (std::vector<XtcFileName>::const_iterator it = files.begin(); it != files.end(); ++ it) {

          const std::string path = it->path();
          const std::string inprog_path = path + ".inprogress";

          if (access(inprog_path.c_str(), R_OK) == 0) {
            MsgLog(logger, debug, "Found file on disk: " << inprog_path);
            found = true;
            break;
          } else if (access(path.c_str(), R_OK) == 0) {
            MsgLog(logger, debug, "Found file on disk: " << path);
            found = true;
            break;
          }
        }
        if (std::time(0) > t0 + m_liveTimeout) break;
        // sleep for one second and repeat
        if (not found) {
          MsgLog(logger, debug, "Wait 1 sec for files to appear on disk");
          sleep(1);
        }
      }

      if (found) {
        // first time we may received incomplete list, update it now
        std::vector<XtcFileName> files = m_filesdb->files(m_expNum, m_run);

        // copy stream numbers from the list
        for (std::vector<XtcFileName>::const_iterator it = files.begin(); it != files.end(); ++ it) {
          MsgLog(logger, debug, "Found stream " << it->stream());
          m_streams.insert(it->stream());
        }

      } else {
        MsgLog(logger, error, "No files appeared on disk after timeout");
        throw XTCLiveTimeout(ERR_LOC, files[0].path(), m_liveTimeout);
      }

      // filter unwanted streams
	  if(m_stream.size() == 1){
		  if (not m_streams.empty() and m_stream[0] != -1) {
			  
			  Streams::iterator it;
			  if (m_stream[0] == -2) {
				  // leave one stream only, try to randomize but in a reproducible way
				  unsigned idx = m_run % m_streams.size();
				  it = m_streams.begin();
				  std::advance(it, idx);
			  } else {
				  // leave one specific stream
				  it = m_streams.find(m_stream[0]);
			  }
			  
			  if (it == m_streams.end()) {
				  m_streams.clear();
				  MsgLog(logger, debug, "One-stream mode, no matching streams");
			  } else {
				  m_streams.erase(m_streams.begin(), it);
				  m_streams.erase(++it, m_streams.end());
				  MsgLog(logger, debug, "One-stream mode, stream number: " << *m_streams.begin());
			  }
		  } // if (not m_streams.empty())

      }else{
		  Streams selectedStreams;
		  Streams::iterator it;
		  for(unsigned i = 0;i<m_stream.size();i++){
			  it = m_streams.find(m_stream[i]);
			  if (it != m_streams.end()){
				  selectedStreams.insert(*it);
			  }
		  }
		  m_streams = selectedStreams;
	  }
      
    } // if (not files.empty())

  } // if (not m_initialized)


  if (not m_streams.empty()) {
    Streams::iterator s = m_streams.begin();
    m_lastStream = *s;
    next = boost::make_shared<ChunkFileIterLive>(m_expNum, m_run, m_lastStream, m_liveTimeout, m_filesdb);
    m_streams.erase(s);
  }

  return next;
}

/**
 *  @brief Return stream number for the set of files returned from last next() call.
 */
unsigned
StreamFileIterLive::stream() const
{
  return m_lastStream;
}

} // namespace XtcInput
