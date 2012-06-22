#ifndef Ami_PimImageComposer_hh
#define Ami_PimImageComposer_hh

#include "ami/event/Composer.hh"

namespace Pds { namespace Lusi { class PimImageConfigV1; } }

namespace Ami {
  class EventHandler;

  class PimImageComposer : public Composer {
  public:
    PimImageComposer(const Pds::Src& info,
		     const void* payload);
  private:
    void   compose(EventHandler&);
  private:
    const Pds::Lusi::PimImageConfigV1& _config;
  };
};

#endif
