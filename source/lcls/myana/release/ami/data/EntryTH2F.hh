#ifndef Pds_ENTRYTH2F_HH
#define Pds_ENTRYTH2F_HH

#include "ami/data/Entry.hh"
#include "ami/data/DescTH2F.hh"

namespace Ami {

  class EntryTH2F : public Entry {
  public:
    EntryTH2F(const Pds::DetInfo& info, unsigned channel,
	      const char* name, const char* xtitle, const char* ytitle,
	      bool isnormalized=false);
    EntryTH2F(const DescTH2F& desc);

    virtual ~EntryTH2F();

    void params(unsigned nbinsx, float xlow, float xup,
		unsigned nbinsy, float ylow, float yup);
    void params(const DescTH2F& desc);

    float content   (unsigned bin) const;
    float content   (unsigned binx, unsigned biny) const;
    void  addcontent(float y, unsigned binx, unsigned biny);
    void  addcontent(float y, double vx, double vy);
    void  content   (float y, unsigned binx, unsigned biny);

    enum Info { UnderflowX, OverflowX, UnderflowY, OverflowY, Normalization, InfoSize };
    float info   (Info) const;
    void  info   (float y, Info);
    void  addinfo(float y, Info);

    void setto(const EntryTH2F& entry);
    void setto(const EntryTH2F& curr, const EntryTH2F& prev);

    // Implements Entry
    virtual const DescTH2F& desc() const;
    virtual DescTH2F& desc();

  private:
    void build(unsigned nbinsx, unsigned nbinsy);

  private:
    DescTH2F _desc;

  private:
    float* _y;
  };

  inline float EntryTH2F::content(unsigned bin) const 
  {
    return *(_y+bin); 
  }
  inline float EntryTH2F::content(unsigned binx, unsigned biny) const 
  {
    return *(_y+binx+biny*_desc.nbinsx()); 
  }
  inline void EntryTH2F::addcontent(float y, unsigned binx, unsigned biny) 
  {
    *(_y+binx+biny*_desc.nbinsx()) += y;
  }
  inline void EntryTH2F::content(float y, unsigned binx, unsigned biny) 
  {
    *(_y+binx+biny*_desc.nbinsx()) = y;
  }

  inline float EntryTH2F::info(Info i) const 
  {
    return *(_y+_desc.nbinsx()*_desc.nbinsy()+int(i));
  }
  inline void EntryTH2F::info(float y, Info i) 
  {
    *(_y+_desc.nbinsx()*_desc.nbinsy()+int(i)) = y;
  }
  inline void EntryTH2F::addinfo(float y, Info i) 
  {
    *(_y+_desc.nbinsx()*_desc.nbinsy()+int(i)) += y;
  }
};

#endif
