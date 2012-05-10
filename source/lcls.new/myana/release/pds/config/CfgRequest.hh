#ifndef Pds_CfgRequest_hh
#define Pds_CfgRequest_hh

#include "pdsdata/xtc/Src.hh"
#include "pdsdata/xtc/TypeId.hh"
#include "pds/utility/Transition.hh"

namespace Pds {

  class CfgRequest {
  public:
    CfgRequest(const Src&        src, 
	       const TypeId&     id,
	       const Transition& tr) :
      _src(src),
      _id (id ),
      _trn(tr )
    {}
  public:
    const Src&        src       () const { return _src; }
    const TypeId&     id        () const { return _id;  }
    const Transition& transition() const { return _trn; }
  private:
    Src        _src;
    TypeId     _id;
    Transition _trn;
  };

}

#endif
