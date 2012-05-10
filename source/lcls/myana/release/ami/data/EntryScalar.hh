#ifndef Pds_ENTRYScalar_HH
#define Pds_ENTRYScalar_HH

#include "ami/data/Entry.hh"
#include "ami/data/DescScalar.hh"

#include <math.h>

namespace Ami {

  class EntryScalar : public Entry {
  public:
    EntryScalar(const Pds::DetInfo& info, unsigned channel, const char* name, const char* ytitle);
    EntryScalar(const DescScalar& desc);

    virtual ~EntryScalar();

    //  The contents are organized as 
    double entries() const;
    double sum    () const;
    double sqsum  () const;
    double mean   () const;
    double rms    () const;
    void   addcontent(double y);

    void setto(const EntryScalar& entry);
    void setto(const EntryScalar& curr, const EntryScalar& prev);
    void add  (const EntryScalar& entry);

    // Implements Entry
    virtual const DescScalar& desc() const;
    virtual DescScalar& desc();

  private:
    DescScalar _desc;

  private:
    double* _y;
  };

  inline double EntryScalar::entries() const { return _y[0]; }
  inline double EntryScalar::sum    () const { return _y[1]; }
  inline double EntryScalar::sqsum  () const { return _y[2]; }
  inline double EntryScalar::mean   () const { return _y[0] ? _y[1]/_y[0] : 0; }
  inline double EntryScalar::rms    () const { return _y[0] ? sqrt((_y[2] - _y[1]*_y[1]/_y[0])/_y[0]) : 0; }
  inline void   EntryScalar::addcontent(double y) { _y[0]++; _y[1]+=y; _y[2]+=y*y; }
};

#endif
