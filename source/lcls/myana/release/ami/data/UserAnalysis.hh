#ifndef UserAnalysis_hh
#define UserAnalysis_hh

namespace Pds {
  class ClockTime;
  class Src;
  class TypeId;
};

namespace Ami {
  class Cds;

  class UserAnalysis {
  public:
    virtual ~UserAnalysis() {}
  public:  // Handler functions
    virtual void reset    () = 0;
    virtual void clock    (const Pds::ClockTime& clk) = 0;
    virtual void configure(const Pds::Src&       src,
			   const Pds::TypeId&    type,
			   void*                 payload) = 0;
    virtual void event    (const Pds::Src&       src,
			   const Pds::TypeId&    type,
			   void*                 payload) = 0;
  public:  // Analysis functions
    virtual void clear    () = 0;     // remove Entry's
    virtual void create   (Cds&) = 0; // create Entry's
    virtual void analyze  () = 0;     // fill   Entry's
  };
};

typedef Ami::UserAnalysis* create_t();
typedef void destroy_t(Ami::UserAnalysis*);

#endif
