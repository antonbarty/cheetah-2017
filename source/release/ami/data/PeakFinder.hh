#ifndef Ami_PeakFinder_hh
#define Ami_PeakFinder_hh

#include "ami/data/AbsOperator.hh"

namespace Ami {

  class Cds;
  class DescEntry;
  class EntryImage;

  //
  //  Peak (hit) finder
  //
  class PeakFinder : public AbsOperator {
  public:
    PeakFinder(unsigned   threshold_value);
    PeakFinder(const char*&, const DescEntry&);
    ~PeakFinder();
  public:
    DescEntry& output   () const;
  private:
    Entry&     _operate  (const Entry&) const;
    void*      _serialize(void*) const;
  private:
    unsigned   _threshold_value;
    EntryImage*       _output_entry;
  };

};

#endif
