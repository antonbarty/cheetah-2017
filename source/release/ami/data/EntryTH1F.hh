#ifndef Pds_ENTRYTH1F_HH
#define Pds_ENTRYTH1F_HH

#include "ami/data/Entry.hh"
#include "ami/data/DescTH1F.hh"

namespace Ami {

  class EntryTH1F : public Entry {
  public:
    EntryTH1F(const Pds::DetInfo& info, unsigned channel, const char* name, const char* xtitle, const char* ytitle);
    EntryTH1F(const DescTH1F& desc);

    virtual ~EntryTH1F();

    void params(unsigned nbins, float xlow, float xup);
    void params(const DescTH1F& desc);

    void clear();

    //  The contents are organized as 
    //  [ data0, data1, ..., dataN-1, underflow, overflow, normalization ]
    double content(unsigned bin) const;
    void   addcontent(double y, unsigned bin);
    void   content(double y, unsigned bin);
    void   addcontent(double y, double x);

    enum Info { Underflow, Overflow, Normalization, InfoSize };
    double info(Info) const;
    void   info(double y, Info);
    void   addinfo(double y, Info);

    void setto(const EntryTH1F& entry);
    void setto(const EntryTH1F& curr, const EntryTH1F& prev);
    void add  (const EntryTH1F& entry);

    // Implements Entry
    virtual const DescTH1F& desc() const;
    virtual DescTH1F& desc();

  private:
    void build(unsigned nbins);

  private:
    DescTH1F _desc;

  private: 
    double* _y;
  };

  inline double EntryTH1F::content   (unsigned bin) const { return *(_y+bin); }
  inline void   EntryTH1F::addcontent(double y, unsigned bin) { *(_y+bin) += y; }
  inline void   EntryTH1F::content   (double y, unsigned bin) { *(_y+bin)  = y; }

  inline double EntryTH1F::info   (Info i) const   {return *(_y+_desc.nbins()+int(i));}
  inline void   EntryTH1F::info   (double y, Info i) {*(_y+_desc.nbins()+int(i)) = y;}
  inline void   EntryTH1F::addinfo(double y, Info i) {*(_y+_desc.nbins()+int(i)) += y;}
};

#endif
