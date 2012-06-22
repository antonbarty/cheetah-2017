#ifndef Ami_Integral_hh
#define Ami_Integral_hh

#include "ami/data/AbsOperator.hh"

namespace Ami {

  class DescEntry;
  class Entry;

  class Integral : public AbsOperator {
  public:
    Integral(const DescEntry&,unsigned xlo,unsigned xhi,unsigned ylo,unsigned yhi);
    Integral(const char*&, const DescEntry&);
    ~Integral();
  public:
    DescEntry& output   () const;
  private:
    Entry&     _operate  (const Entry&) const;
    void*      _serialize(void*) const;
  private:
    Entry*         _entry;
    unsigned       _xlo, _xhi, _ylo, _yhi;
  };

};

#endif
