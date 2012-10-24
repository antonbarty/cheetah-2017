//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: ExpNameFromXtc.cpp 3038 2012-03-08 22:12:17Z salnikov@SLAC.STANFORD.EDU $
//
// Description:
//	Class ExpNameFromXtc...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana/ExpNameFromXtc.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <stdlib.h>
#include <fstream>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "ExpNameDb/ExpNameDatabase.h"
#include "MsgLogger/MsgLogger.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

using namespace std;

namespace {

  const char* logger = "ExpNameFromXtc";

  // get base name
  string basename(const string& name)
  {
    // remove dirname
    string::size_type n = name.rfind('/') ;
    if ( n != string::npos ) return string(name, n+1) ;
    return name ;
  }

  // extract experiment number from file name which must have format eNN-r.....xtc,
  // return negative number if cannot parse file name
  int expNumber(const string& path)
  {
    string name = basename(path);

    // strip everything after exp number
    string::size_type n = name.find('-') ;
    if (n == string::npos) return -1;
    name.erase(n);

    // parse eNN
    if ( name.size() < 2 || name[0] != 'e' ) return -1;
    char *eptr = 0 ;
    int val = strtol ( name.c_str()+1, &eptr, 10 ) ;
    if (*eptr != '\0') return -1;
    return val;
  }

}

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana {

//----------------
// Constructors --
//----------------
ExpNameFromXtc::ExpNameFromXtc (const std::list<std::string>& files)
  : IExpNameProvider()
  , m_instr()
  , m_exp()
  , m_expNum(0)
{
  // extract exp number for every file name, they all must be the same
  for (list<string>::const_iterator it = files.begin(); it != files.end(); ++ it) {
    int exp = ::expNumber(*it);
    if (exp < 0) {
      MsgLog(logger, warning, "ExpNameFromXtc: file name " << *it << " has no valid experiment number");
      break;
    }
    if (m_expNum == 0) {
      m_expNum = exp;
    } else if (unsigned(exp) != m_expNum) {
      WithMsgLog(logger, warning, out ) {
        out << "ExpNameFromXtc: XTC files belong to different experiments:";
        for (list<string>::const_iterator it = files.begin(); it != files.end(); ++ it) {
          out << "\n    " << *it;
        }
      }
      break;
    }
  }
  if (m_expNum == 0) return;

  ExpNameDb::ExpNameDatabase namedb;
  const pair<string, string>& res = namedb.getNames(m_expNum);
  if (res.first.empty()) {
    MsgLog(logger, warning, "ExpNameFromXtc: failed to find experiment number " << m_expNum);
  }
  m_instr = res.first;
  m_exp = res.second;
}

//--------------
// Destructor --
//--------------
ExpNameFromXtc::~ExpNameFromXtc ()
{
}

} // namespace psana
