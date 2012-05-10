#ifndef Ami_XYHistogram_hh
#define Ami_XYHistogram_hh

//
//  class XYHistogram : an operator that generates a histogram of
//    an EntryImage pixel values.
//

#include "ami/data/AbsOperator.hh"
#include "ami/data/DescTH1F.hh"

namespace Ami {

  class DescEntry;
  class Entry;

  class XYHistogram : public AbsOperator {
  public:
    XYHistogram(const DescEntry& output,
                double xlo, double xhi,
                double ylo, double yhi);
    XYHistogram(const char*&, const DescEntry&);
    XYHistogram(const char*&);
    ~XYHistogram();
  public:
    DescEntry& output   () const;
  private:
    Entry&     _operate  (const Entry&) const;
    void*      _serialize(void*) const;
  private:
    enum { DESC_LEN = sizeof(DescTH1F) };
    char             _desc_buffer[DESC_LEN];
    double           _xlo;
    double           _xhi;
    double           _ylo;
    double           _yhi;
    Entry*           _output;
  };

};

#endif
