#ifndef UserFilter_hh
#define UserFilter_hh

#include "pdsdata/xtc/ClockTime.hh"
#include "pdsdata/xtc/Src.hh"
#include "pdsdata/xtc/TypeId.hh"

#include "ami/data/FeatureCache.hh"

namespace Ami {
  class UserFilter {
  public:
    virtual ~UserFilter() {}
  public:  // Handler functions
    virtual void reset    (FeatureCache&) = 0;
    virtual void clock    (const Pds::ClockTime& clk) = 0;
    virtual void configure(const Pds::Src&       src,
			   const Pds::TypeId&    type,
			   void*                 payload) = 0;
    virtual void event    (const Pds::Src&       src,
			   const Pds::TypeId&    type,
			   void*                 payload) = 0;
  public:
    virtual const char* name() const = 0;
    virtual bool accept () = 0;
  };
};

typedef Ami::UserFilter* create_f();

#endif
