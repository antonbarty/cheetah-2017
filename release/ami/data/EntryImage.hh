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
    unsigned info   (Info) const;
    void     info   (unsigned y, Info);
    void     addinfo(unsigned y, Info);

    void setto(const EntryImage& entry);
    void setto(const EntryImage& curr, const EntryImage& prev);
    void add  (const EntryImage& entry);

    // Implements Entry
    virtual const DescImage& desc() const;
    virtual DescImage& desc();

  private:
    void build(unsigned nbinsx, unsigned nbinsy);

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
  inline unsigned EntryImage::info(Info i) const 
  {
    return *(_y+_desc.nbinsx()*_desc.nbinsy()+int(i));
  }
  inline void EntryImage::info(unsigned y, Info i) 
  {
    *(_y+_desc.nbinsx()*_desc.nbinsy()+int(i)) = y;
  }
  inline void EntryImage::addinfo(unsigned y, Info i) 
  {
    *(_y+_desc.nbinsx()*_desc.nbinsy()+int(i)) += y;
  }
};

#endif
