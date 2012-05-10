#ifndef Ami_Factory_hh
#define Ami_Factory_hh

class iovec;
#include <vector>

namespace Ami {

  class Cds;
  class FeatureCache;
  class Message;

  class Factory {
  public:
    virtual ~Factory() {}
  public:
    virtual std::vector<FeatureCache*>& features() = 0;
    virtual Cds& discovery() = 0;
    virtual Cds& hidden   () = 0;
    virtual void configure(unsigned id, const Message&, const char*, Cds&) = 0;
    virtual void analyze  () = 0;
    virtual void discover (bool waitForConfigure) = 0;
    virtual void remove   (unsigned id) = 0;
  };

};

#endif
