#ifndef Ami_RPhiProjection_hh
#define Ami_RPhiProjection_hh

//
//  class RPhiProjection : an operator that projects an EntryImage onto an axis
//    (RPhiProjection::Axis) over a range of the other axis.
//

#include "ami/data/AbsOperator.hh"
#include "ami/data/DescProf.hh"

namespace Ami {

  class DescEntry;
  class Entry;

  class RPhiProjection : public AbsOperator {
  public:
    enum Axis { R, Phi };
    RPhiProjection(const DescEntry& output,
		   Axis, double lo, double hi,
		   double xc, double yc);
    RPhiProjection(const char*&, const DescEntry&);
    RPhiProjection(const char*&);
    ~RPhiProjection();
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
    double           _xc;
    double           _yc;
    Entry*           _output;
  };

};

#endif
