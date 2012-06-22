#ifndef Pds_ENTRYImage_HH
#define Pds_ENTRYImage_HH

#include "ami/data/Entry.hh"
#include "ami/data/DescImage.hh"

namespace Ami {

  class EntryImage : public Entry {
  public:
    EntryImage(const Pds::DetInfo&, unsigned channel, const char* name);
    EntryImage(const DescImage& desc);

    virtual ~EntryImage();

    void params(unsigned nbinsx,
		unsigned nbinsy,
		int ppxbin,
		int ppybin);
    void params(const DescImage& desc);

    unsigned content   (unsigned bin) const;
    unsigned content   (unsigned binx, unsigned biny) const;
    void     addcontent(unsigned y, unsigned binx, unsigned biny);
    void     content   (unsigned y, unsigned binx, unsigned biny);

    unsigned*       contents();
    const unsigned* contents() const;

    enum Info { Pedestal, Normalization, InfoSize };
    double   info   (Info) const;
    void     info   (double y, Info);
    void     addinfo(double y, Info);

    void setto(const EntryImage& entry);
    void setto(const EntryImage& curr, const EntryImage& prev);
    void add  (const EntryImage& entry);

    // Implements Entry
    virtual const DescImage& desc() const;
    virtual DescImage& desc();

  private:
    void build();

  private:
    DescImage _desc;

  private:
    unsigned* _y;
  };

  inline unsigned EntryImage::content(unsigned bin) const 
  {
    return *(_y+bin); 
  }
  inline unsigned EntryImage::content(unsigned binx, unsigned biny) const 
  {
    return *(_y+binx+biny*_desc.nbinsx()); 
  }
  inline void EntryImage::addcontent(unsigned y, unsigned binx, unsigned biny) 
  {
    *(_y+binx+biny*_desc.nbinsx()) += y;
  }
  inline void EntryImage::content(unsigned y, unsigned binx, unsigned biny) 
  {
    *(_y+binx+biny*_desc.nbinsx()) = y;
  }
  inline unsigned* EntryImage::contents()
  {
    return _y;
  }
  inline const unsigned* EntryImage::contents() const
  {
    return _y;
  }
  inline double EntryImage::info(Info i) const 
  {
    return reinterpret_cast<float*>(_y+_desc.nbinsx()*_desc.nbinsy())[i];
  }
  inline void EntryImage::info(double y, Info i) 
  {
    reinterpret_cast<float*>(_y+_desc.nbinsx()*_desc.nbinsy())[i] = y;
  }
  inline void EntryImage::addinfo(double y, Info i) 
  {
    reinterpret_cast<float*>(_y+_desc.nbinsx()*_desc.nbinsy())[i] += y;
  }
};

#endif
