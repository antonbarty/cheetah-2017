#ifndef Pds_CfgPath_hh
#define Pds_CfgPath_hh

#include "pdsdata/xtc/TypeId.hh"

#include <string>
using std::string;
#include <sstream>
using std::ostringstream;
#include <iomanip>
using std::setw;
using std::setfill;

namespace Pds {

  class Src;
  class TypeId;

  class CfgPath {
  public:
    static std::string src_key(const Src& src);
    static std::string path(unsigned      key,
			    const Src&    src,
			    const TypeId& type);
  };

  inline string CfgPath::path(unsigned key, const Src& src, const TypeId& type)
  {
    ostringstream o;
    o << std::hex << setfill('0') << setw(8) << key << '/' 
      << CfgPath::src_key(src) << '/' 
      << setw(8) << type.value();
    return o.str();
  }


  inline string CfgPath::src_key(const Src& src) {
    ostringstream o;
    if (src.level()==Level::Source) {
      o << std::hex << setfill('0') << setw(8) << src.phy();
    }
    else {
      o << std::hex << setw(1) << src.level();
    }
    return o.str();
  }

};

#endif
