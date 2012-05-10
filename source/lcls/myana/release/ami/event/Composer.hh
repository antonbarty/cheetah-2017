#ifndef Ami_Composer_hh
#define Ami_Composer_hh

#include "pdsdata/xtc/Src.hh"

namespace Ami {
  class EventHandler;

  class Composer {
  public:
    Composer(const Pds::Src&     info);
    virtual ~Composer();
  public:
    virtual void compose(EventHandler&) = 0;
  public:
    const Pds::Src& info() const { return _info; }
  private:
    Pds::Src _info;
  };
};

#endif
