#ifndef Pds_EntryScan_hh
#define Pds_EntryScan_hh

#include <stdio.h>
#include <math.h>

#include "ami/data/Entry.hh"
#include "ami/data/DescScan.hh"

namespace Ami {

  class EntryScan : public Entry {
  public:
    EntryScan(const Pds::DetInfo& info, unsigned channel, const char* name, const char* xtitle, const char* ytitle);
    EntryScan(const DescScan& desc);

    virtual ~EntryScan();

    void params(unsigned nbins);
    void params(const DescScan& desc);

    double ymean(unsigned bin) const;
    double sigma(unsigned bin) const;

    double ysum(unsigned bin) const;
    void ysum(double y, unsigned bin);

    double y2sum(unsigned bin) const;
    void y2sum(double y2, unsigned bin);

    double nentries(unsigned bin) const;
    void nentries(double nent, unsigned bin);

    double xbin(unsigned) const;
    void   xbin(double, unsigned bin);

    void addy(double y, unsigned bin, double w=1);
    void addy(double y, double x, double w=1);

    enum Info { Current, Normalization, InfoSize };
    double info(Info) const;
    void   info(double, Info);
    void   addinfo(double, Info);

    void setto(const EntryScan& entry);
    void sum  (const double*, const double*);
    void sum  (const EntryScan&, const EntryScan&);
    void diff (const EntryScan&, const EntryScan&);
    void add  (const EntryScan& entry);

    // Implements Entry
    virtual const DescScan& desc() const;
    virtual DescScan& desc();

  private:
    void build(unsigned nbins);

  private:
    DescScan _desc;

  private:
    class BinV { public: double _x; double _nentries; double _ysum; double _y2sum; };
    BinV* _p;

    int  _insert_bin(const BinV&, int&);
  };

  inline double EntryScan::ymean(unsigned bin) const 
  {
    double n = nentries(bin);
    if (n > 0) {
      double y = ysum(bin);
      double mean = y/n;
      return mean;
    } else {
      return 0;
    }
  }

  inline double EntryScan::sigma(unsigned bin) const 
  {
    double n = nentries(bin);
    if (n > 0) {
      double y = ysum(bin);
      double y2 = y2sum(bin);
      double mean = y/n;
      double s = sqrt(y2/n-mean*mean);
      return s;
    } else {
      return 0;
    }
  }

  inline double EntryScan::ysum(unsigned bin) const {return _p[bin]._ysum;}
  inline void EntryScan::ysum(double y, unsigned bin) {_p[bin]._ysum=y;}

  inline double EntryScan::y2sum(unsigned bin) const {return _p[bin]._y2sum;}
  inline void EntryScan::y2sum(double y2, unsigned bin) {_p[bin]._y2sum=y2;}

  inline double EntryScan::nentries(unsigned bin) const {return _p[bin]._nentries;}
  inline void EntryScan::nentries(double nent, unsigned bin) {_p[bin]._nentries=nent;}

  inline void EntryScan::addy(double y, unsigned bin, double w) 
  {
    _p[bin]._nentries += w;
    _p[bin]._ysum     += y*w;
    _p[bin]._y2sum    += y*y*w;
  }

  inline double EntryScan::xbin(unsigned bin) const {return _p[bin]._x;}
  inline void   EntryScan::xbin(double v, unsigned bin) {_p[bin]._x=v;}

  inline double EntryScan::info(Info i) const { return reinterpret_cast<double*>(_p+_desc.nbins())[i]; }
  inline void   EntryScan::info(double y,Info i) { reinterpret_cast<double*>(_p+_desc.nbins())[i]=y; }
  inline void   EntryScan::addinfo(double y,Info i) { reinterpret_cast<double*>(_p+_desc.nbins())[i]+=y; }
};

#endif
