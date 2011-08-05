#ifndef Ami_AbsOperator_HH
#define Ami_AbsOperator_HH

//
//  The AbsOperator class is a base class for objects which operate on
//  an input Entry and produce an output Entry.  The constructor should
//  create the output Entry.  Objects of this class are transmitted
//  over the network, which is achieved using the "serialize" member and
//  a corresponding constructor (derived classes) to reconstruct the object.
//

#include <stdint.h>

namespace Ami {

  class DescEntry;
  class Entry;

  class AbsOperator {
  public:
    enum Type { Single, Average, Mean, Integral, Value, 
		XYProjection, RPhiProjection, 
		Reference, EntryMath, BinMath, EdgeFinder, PeakFinder, 
		EnvPlot, PeakFitPlot, FFT, ContourProjection, TdcPlot, 
                XYHistogram, Zoom };
    AbsOperator(Type t);
    virtual ~AbsOperator() {}
    Entry&       operator ()(const Entry&) const;
    void*        serialize(void* p) const;
    Type         type() const;
    AbsOperator* next() const;
    void         next(AbsOperator* o);
  public:
    virtual DescEntry& output   () const = 0;
  private:
    virtual Entry& _operate  (const Entry&) const = 0;
    virtual void*  _serialize(void* p     ) const = 0;
  protected:
    //  helper functions for (de)serialization
    void _insert (void*& p, const void* b, unsigned size) const;
    void _extract(const char*& p, void* b, unsigned size);
  private:
    uint32_t     _type;
    AbsOperator* _next;
  };
};

#endif
