#ifndef Ami_EdgeFinder_hh
#define Ami_EdgeFinder_hh

#include "ami/data/AbsOperator.hh"

#include "ami/data/DescTH1F.hh"

namespace Ami {

  class DescEntry;
  class EntryWaveform;
  class EntryTH1F;

  //
  //  Leading edge finder
  //
  class EdgeFinder : public AbsOperator {
  public:
    EdgeFinder(double     fraction,
	       double     threshold_value,
	       double     baseline_value,
	       const      DescTH1F& output);
    EdgeFinder(const char*&);
    ~EdgeFinder();
  public:
    DescEntry& output   () const;
  private:
    Entry&     _operate  (const Entry&) const;
    void*      _serialize(void*) const;
  private:
    double     _fraction;
    double     _threshold_value;
    double     _baseline_value;
    DescTH1F   _output;
    EntryTH1F*           _output_entry;
  };

};

#endif
