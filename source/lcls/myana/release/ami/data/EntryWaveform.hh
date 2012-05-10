#ifndef Pds_ENTRYWaveform_HH
#define Pds_ENTRYWaveform_HH

#include "ami/data/Entry.hh"
#include "ami/data/DescWaveform.hh"

namespace Ami {

  class EntryWaveform : public Entry {
  public:
    EntryWaveform(const Pds::DetInfo& info, unsigned channel,
		  const char* name, const char* xtitle, const char* ytitle);
    EntryWaveform(const DescWaveform& desc);

    virtual ~EntryWaveform();

    void params(unsigned nbins, float xlow, float xup);
    void params(const DescWaveform& desc);

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

    void setto(const EntryWaveform& entry);
    void setto(const EntryWaveform& curr, const EntryWaveform& prev);
    void add  (const EntryWaveform& entry);

    // Implements Entry
    virtual const DescWaveform& desc() const;
    virtual DescWaveform& desc();

  private:
    void build(unsigned nbins);

  private:
    DescWaveform _desc;

  private:
    double* _y;
  };

  inline double EntryWaveform::content   (unsigned bin) const { return *(_y+bin); }
  inline void   EntryWaveform::addcontent(double y, unsigned bin) { *(_y+bin) += y; }
  inline void   EntryWaveform::content   (double y, unsigned bin) { *(_y+bin)  = y; }

  inline double EntryWaveform::info   (Info i) const   {return *(_y+_desc.nbins()+int(i));}
  inline void   EntryWaveform::info   (double y, Info i) {*(_y+_desc.nbins()+int(i)) = y;}
  inline void   EntryWaveform::addinfo(double y, Info i) {*(_y+_desc.nbins()+int(i)) += y;}
};

#endif
