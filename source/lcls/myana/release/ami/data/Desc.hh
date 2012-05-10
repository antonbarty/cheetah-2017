#ifndef Pds_DESC_HH
#define Pds_DESC_HH

#include <stdint.h>

namespace Ami {

  class Desc {
  public:
    Desc(const char* name);
    Desc(const Desc& desc);
    ~Desc();

    const char* name() const;
    int signature() const;
    unsigned nentries() const;
    void nentries(unsigned);
    void added();
    void reset();
    void signature(int i);

  private:
    enum {NameSize=128};
    char     _name[NameSize];
    int32_t  _signature;
    uint32_t _nentries;
  };
};

#endif
