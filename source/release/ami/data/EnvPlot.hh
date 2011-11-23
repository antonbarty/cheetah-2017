#ifndef Ami_EnvPlot_hh
#define Ami_EnvPlot_hh

//
//  class EnvPlot : an operator that fetches a BLD or PV quantity
//    and generates a mean value (Scalar), a distribution (TH1F), or a profile (Prof)
//    versus a BLD or PV quantity.
//

#include "ami/data/AbsOperator.hh"

namespace Ami {

  class Cds;
  class DescEntry;
  class Entry;
  class Term;
  class FeatureCache;

  class EnvPlot : public AbsOperator {
  public:
    //  Defined by the output entry's description,
    //    the BLD/PV, and any BLD/PV dependence for profiles.
    EnvPlot(const DescEntry& output);
    //  Reconstituted from the input serial stream, the BldState and PvState
    //    accessors, and the Cds input entry accessor.
    EnvPlot(const char*&, FeatureCache&, const Cds&);
    ~EnvPlot();
  public:
    DescEntry& output   () const;
  private:
    Entry&     _operate  (const Entry&) const;
    void*      _serialize(void*) const;
  private:
    enum { DESC_LEN = 1024 };
    char             _desc_buffer[DESC_LEN];

    FeatureCache* _cache;
    Term*         _term;
    Term*         _weight;
    Entry*        _entry;
    Term*         _input;
  };

};

#endif
