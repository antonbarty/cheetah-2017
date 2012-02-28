#ifndef Pds_DescImage_HH
#define Pds_DescImage_HH

#include "ami/data/DescEntry.hh"

namespace Ami {

  class SubFrame {
  public:
    uint16_t x;
    uint16_t y;
    uint16_t nx;
    uint16_t ny;
  };

  class DescImage : public DescEntry {
  public:
    DescImage(const Pds::DetInfo& info,
	      const char* name,
              const char* zunits,
	      unsigned nbinsx, unsigned nbinsy, 
	      int ppbx=1, int ppby=1,  // pixels per bin
              bool pedCalib=false, bool gainCalib=false, bool rmsCalib=false);

    DescImage(const Pds::DetInfo& info,
	      unsigned channel,
	      const char* name,
	      unsigned nbinsx, unsigned nbinsy, 
	      int ppbx=1, int ppby=1); // pixels per bin

    DescImage(const char* name,
	      unsigned nbinsx, unsigned nbinsy, 
	      int ppbx=1, int ppby=1,  // pixels per bin
              unsigned xp0=0, unsigned yp0=0);  // pixel origin

    unsigned nbinsx() const;
    unsigned nbinsy() const;
    int ppxbin() const;
    int ppybin() const;
    float xlow() const;
    float xup() const;
    float ylow() const;
    float yup() const;
    int xbin(float) const;
    int ybin(float) const;
    float binx(unsigned) const;
    float biny(unsigned) const;

    float mmppx() const { return _mmppx; }
    float mmppy() const { return _mmppy; }

    void params(unsigned nbinsx,
		unsigned nbinsy,
		int ppxbin,
		int ppybin);

    void set_scale(float mmppx,
		   float mmppy);

    void add_frame(unsigned x,
		   unsigned y,
		   unsigned nx,
		   unsigned ny);

    unsigned nframes() const;

    const SubFrame& frame(unsigned) const;

    bool xy_bounds(int& x0, int& x1, int& y0, int& y1) const;
    bool xy_bounds(int& x0, int& x1, int& y0, int& y1, unsigned frame) const;

    bool rphi_bounds(int& x0, int& x1, int& y0, int& y1,
		     double xc, double yc, double r) const;
    bool rphi_bounds(int& x0, int& x1, int& y0, int& y1,
		     double xc, double yc, double r, unsigned frame) const;

  private:
    uint16_t _nbinsx;
    uint16_t _nbinsy;
    int16_t  _ppbx;
    int16_t  _ppby;
    int32_t  _xp0;
    int32_t  _yp0;
    float    _mmppx;
    float    _mmppy;
    enum { MAX_SUBFRAMES=64 };
    uint32_t _nsubframes;
    SubFrame _subframes[MAX_SUBFRAMES];
  };

  inline unsigned DescImage::nbinsx() const {return _nbinsx;}
  inline unsigned DescImage::nbinsy() const {return _nbinsy;}
  inline int DescImage::ppxbin() const {return _ppbx;}
  inline int DescImage::ppybin() const {return _ppby;}
  inline float DescImage::xlow() const {return _xp0;}
  inline float DescImage::xup () const {return _xp0+_nbinsx*_ppbx;}
  inline float DescImage::ylow() const {return _yp0;}
  inline float DescImage::yup () const {return _yp0+_nbinsy*_ppby;}
  inline int DescImage::xbin(float x) const {return (int(x)-_xp0)/_ppbx;}
  inline int DescImage::ybin(float y) const {return (int(y)-_yp0)/_ppby;}
  inline float DescImage::binx(unsigned b) const {return _xp0+float(b)*_ppbx;}
  inline float DescImage::biny(unsigned b) const {return _yp0+float(b)*_ppby;}
  inline unsigned DescImage::nframes() const { return _nsubframes; }
  inline const SubFrame& DescImage::frame(unsigned i) const { return _subframes[i]; }
};

#endif
