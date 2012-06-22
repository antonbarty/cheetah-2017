#ifndef Ami_PVState
#define Ami_PVState

#include "Exception.hh"

#include <map>

namespace Ami {

  class Ca;

  class CaNameCompare {
  public:
    bool operator()(const Ca*, const Ca*) const;
  };

  class PVState {
  public:
    PVState();
    ~PVState();
  public:
    double value(const Ca*) const throw(Event);
  public:
    void add(const Ca&);
  private:
    typedef  std::map <const Ca* , double, CaNameCompare>  MapType;
    typedef  std::pair<const Ca* , double> ElType;
    MapType  _map;
  };

};

#endif
