#ifndef Pds_ENTRYPROF_HH
#define Pds_ENTRYPROF_HH

#include <stdio.h>
#include <math.h>

#include "ami/data/Entry.hh"
#include "ami/data/DescProf.hh"

namespace Ami {

  class EntryProf : public Entry {
  public:
    EntryProf(const Pds::DetInfo& info, unsigned channel, const char* name, const char* xtitle, const char* ytitle);
    EntryProf(const DescProf& desc);

    virtual ~EntryProf();

    void params(unsigned nbins, float xlow, float xup, const char* names);
    void params(const DescProf& desc);

    double ymean(unsigned bin) const;
    double sigma(unsigned bin) const;

    double ysum(unsigned bin) const;
    void ysum(double y, unsigned bin);

    double y2sum(unsigned bin) const;
    void y2sum(double y2, unsigned bin);

    double nentries(unsigned bin) const;
    void nentries(double nent, unsigned bin);

    void addy(double y, unsigned bin);
    void addy(double y, double x);

    enum Info { Underflow, Overflow, Normalization, InfoSize };
    double info(Info) const;
    void   info(double, Info);
    void   addinfo(double, Info);

    void setto(const EntryProf& entry);
    void sum  (const EntryProf&, const EntryProf&);
    void diff (const EntryProf&, const EntryProf&);

    // Implements Entry
    virtual const DescProf& desc() const;
    virtual DescProf& desc();

  private:
    void build(unsigned nbins);

  private:
    DescProf _desc;

  private:
    double* _ysum;
    double* _y2sum;
    double* _nentries;
  };

  inline double EntryProf::ymean(unsigned bin) const 
  {
    double n = *(_nentries+bin);
    if (n > 0) {
      double y = *(_ysum+bin);
      double mean = y/n;
      return mean;
    } else {
      return 0;
    }
  }

  inline double EntryProf::sigma(unsigned bin) const 
  {
    double n = *(_nentries+bin);
    if (n > 0) {
      double y = *(_ysum+bin);
      double y2 = *(_y2sum+bin);
      double mean = y/n;
      double s = sqrt(y2/n-mean*mean);
      return s;
    } else {
      return 0;
    }
  }

  inline double EntryProf::ysum(unsigned bin) const {return *(_ysum+bin);}
  inline void EntryProf::ysum(double y, unsigned bin) {*(_ysum+bin) = y;}

  inline double EntryProf::y2sum(unsigned bin) const {return *(_y2sum+bin);}
  inline void EntryProf::y2sum(double y2, unsigned bin) {*(_y2sum+bin) = y2;}

  inline double EntryProf::nentries(unsigned bin) const {return *(_nentries+bin);}
  inline void EntryProf::nentries(double nent, unsigned bin) {*(_nentries+bin) = nent;}

  inline void EntryProf::addy(double y, unsigned bin) 
  {
    _ysum[bin] += y;
    _y2sum[bin] += y*y;
    _nentries[bin] += 1;
  }
  inline void EntryProf::addy(double y, double x)
  {
    int bin = int((x-_desc.xlow())*double(_desc.nbins())/(_desc.xup()-_desc.xlow()));
    if (bin < 0)      
      addinfo(y,Underflow);
    else if ((unsigned)bin >= _desc.nbins())
      addinfo(y,Overflow);
    else 
      addy   (y,(unsigned)bin);
  }

  inline double EntryProf::info(Info i) const { return *(_nentries+_desc.nbins()+int(i)); }
  inline void   EntryProf::info(double y,Info i) { *(_nentries+_desc.nbins()+int(i)) = y; }
  inline void   EntryProf::addinfo(double y,Info i) { *(_nentries+_desc.nbins()+int(i)) += y; }
};

#endif
