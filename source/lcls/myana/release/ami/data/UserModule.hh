#ifndef UserModule_hh
#define UserModule_hh

#include "pdsdata/xtc/ClockTime.hh"
#include "pdsdata/xtc/Src.hh"
#include "pdsdata/xtc/TypeId.hh"

#include "ami/data/Cds.hh"
#include "ami/data/FeatureCache.hh"

namespace Ami {
  class UserModule {
  public:
    virtual ~UserModule() {}
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
    virtual bool accept () { return true; }
  public:
    virtual void clear (    ) = 0;
    virtual void create(Cds&) = 0;
  };
};

typedef Ami::UserModule* create_m();
typedef void destroy_m(Ami::UserModule*);

#endif
