#ifndef Ami_XYProjection_hh
#define Ami_XYProjection_hh

//
//  class XYProjection : an operator that projects an EntryImage onto an axis
//    (XYProjection::Axis) over a range of the other axis.
//

#include "ami/data/AbsOperator.hh"
#include "ami/data/DescProf.hh"

namespace Ami {

  class DescEntry;
  class Entry;

  class XYProjection : public AbsOperator {
  public:
    enum Axis { X, Y };
    XYProjection(const DescEntry& output,
		 Axis, double lo, double hi);
    XYProjection(const char*&, const DescEntry&);
    XYProjection(const char*&);
    ~XYProjection();
  public:
    DescEntry& output   () const;
  private:
    Entry&     _operate  (const Entry&) const;
    void*      _serialize(void*) const;
  private:
    enum { DESC_LEN = sizeof(DescProf) };
    char             _desc_buffer[DESC_LEN];
    Axis             _axis;
    double           _lo;
    double           _hi;
    Entry*           _output;
  };

};

#endif
