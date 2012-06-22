#ifndef Ami_Src_hh
#define Ami_Src_hh

#include "pdsdata/xtc/Src.hh"

namespace Ami {
  class Src : public Pds::Src {
  public:
    Src (unsigned id);
    ~Src();
  public:
    unsigned id() const;
  };
};

#endif
