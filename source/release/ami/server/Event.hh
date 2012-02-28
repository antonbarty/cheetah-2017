#ifndef Ami_Event
#define Ami_Event

#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/Xtc.hh"
#include "Exception.hh"

#include <map>

namespace Ami {

  class Event {
  public:
    Event();
    ~Event();
  public:
    const Pds::Xtc& value(const Pds::DetInfo&) const throw(NoValue);
  public:
    void add(const Pds::Xtc&);
  private:
    typedef std::map <Pds::DetInfo, const Pds::Xtc*, DetInfoCompare> MapType;
    typedef  std::pair<Pds::DetInfo, const Pds::Xtc*> ElType;
    MapType _map;
  };

};

#endif
