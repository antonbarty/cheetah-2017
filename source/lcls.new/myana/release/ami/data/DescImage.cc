#include "ami/data/DescImage.hh"

#include <stdio.h>

using namespace Ami;

DescImage::DescImage(const char* name, 
		     unsigned nbinsx, 
		     unsigned nbinsy, 
		     int ppbx,
		     int ppby,
                     unsigned xp0,
                     unsigned yp0,
                     bool isnormalized) :
  DescEntry(name, "x", "y", Image, sizeof(DescImage), isnormalized),
  _nbinsx(nbinsx ? nbinsx : 1),
  _nbinsy(nbinsy ? nbinsy : 1),
  _ppbx  (ppbx),
  _ppby  (ppby),
  _xp0   (xp0),
  _yp0   (yp0),
  _mmppx (0),
  _mmppy (1),
  _nsubframes(0)
{}

DescImage::DescImage(const Pds::DetInfo& info,
		     unsigned channel,
		     const char* name, 
		     unsigned nbinsx, 
		     unsigned nbinsy, 
		     int ppbx,
		     int ppby) :
  DescEntry(info, channel, name, "x", "y", Image, sizeof(DescImage)),
  _nbinsx(nbinsx ? nbinsx : 1),
  _nbinsy(nbinsy ? nbinsy : 1),
  _ppbx  (ppbx),
  _ppby  (ppby),
  _xp0   (0),
  _yp0   (0),
  _mmppx (0),
  _mmppy (1),
  _nsubframes(0)
{}

DescImage::DescImage(const Pds::DetInfo& info,
		     const char* name, 
                     const char* zunits,
		     unsigned nbinsx, 
		     unsigned nbinsy, 
		     int ppbx,
		     int ppby,
                     bool pedCalib,
                     bool gainCalib,
                     bool rmsCalib) :
  DescEntry(info, 0, name, "x", "y", zunits, 
            Image, sizeof(DescImage),
            true, true, 
            pedCalib, gainCalib, rmsCalib),
  _nbinsx(nbinsx ? nbinsx : 1),
  _nbinsy(nbinsy ? nbinsy : 1),
  _ppbx  (ppbx),
  _ppby  (ppby),
  _xp0   (0),
  _yp0   (0),
  _mmppx (0),
  _mmppy (1),
  _nsubframes(0)
{}

void DescImage::params(unsigned nbinsx,
		       unsigned nbinsy,
		       int ppxbin,
		       int ppybin)
{
  _nbinsx = nbinsx ? nbinsx : 1;
  _nbinsy = nbinsy ? nbinsy : 1;
  _ppbx = ppxbin;
  _ppby = ppybin;
}

void DescImage::set_scale(float scalex, 
			  float scaley)
{
  _mmppx = scalex;
  _mmppy = scaley;
}

void DescImage::add_frame(unsigned x,
			  unsigned y,
			  unsigned nx,
			  unsigned ny)
{
  if (_nsubframes < MAX_SUBFRAMES) {
    unsigned frame = _nsubframes++;
    _subframes[frame].x  = x;
    _subframes[frame].y  = y;
    _subframes[frame].nx = nx;
    _subframes[frame].ny = ny;
  }
  else {
    printf("DescImage::add_frame already at maximum (%d)\n",
	   MAX_SUBFRAMES);
  }
}

bool DescImage::xy_bounds(int& x0, int& x1, int& y0, int& y1) const
{
  if ((x0 >= (int)nbinsx()) ||
      (x1 < 0) ||
      (y0 >= (int)nbinsy()) ||
      (y1 < 0))
    return false;
  if (x0 < 0) x0=0;
  if (x1 > (int)nbinsx()) x1=nbinsx();
  if (y0 < 0) y0=0;
  if (y1 > (int)nbinsy()) y1=nbinsy();
  return true;
}

bool DescImage::xy_bounds(int& x0, int& x1, int& y0, int& y1, unsigned fn) const
{
  const SubFrame& f = frame(fn);
  x0 -= f.x;
  x1 -= f.x;
  y0 -= f.y;
  y1 -= f.y;
  if ((x0 >= (int)f.nx) ||
      (x1 < 0) ||
      (y0 >= (int)f.ny) ||
      (y1 < 0))
    return false;
  if (x0 < 0) x0=0;
  if (x1 > (int)f.nx) x1=f.nx;
  if (y0 < 0) y0=0;
  if (y1 > (int)f.ny) y1=f.ny;
  x0 += f.x;
  x1 += f.x;
  y0 += f.y;
  y1 += f.y;
  return true;
}

bool DescImage::rphi_bounds(int& x0, int& x1, int& y0, int& y1,
			    double xc, double yc, double r) const
{
  x0 = xbin(xc - r);
  x1 = xbin(xc + r);
  y0 = ybin(yc - r);
  y1 = ybin(yc + r);
  if ((x0 >= (int)nbinsx()) ||
      (x1 < 0) ||
      (y0 >= (int)nbinsy()) || 
      (y1 < 0))
    return false;
  if (x0 < 0) x0=0;
  if (x1 > (int)nbinsx()) x1=nbinsx();
  if (y0 < 0) y0=0;
  if (y1 > (int)nbinsy()) y1=nbinsy();
  return true;
}

bool DescImage::rphi_bounds(int& x0, int& x1, int& y0, int& y1,
			    double xc, double yc, double r, unsigned fn) const
{
  const SubFrame& f = frame(fn);
  x0 = xbin(xc - r) - f.x;
  x1 = xbin(xc + r) - f.x;
  y0 = ybin(yc - r) - f.y;
  y1 = ybin(yc + r) - f.y;
  if ((x0 >= (int)f.nx) ||
      (x1 < 0) ||
      (y0 >= (int)f.ny) || 
      (y1 < 0))
    return false;
  if (x0 < 0) x0=0;
  if (x1 > (int)f.nx) x1=f.nx;
  if (y0 < 0) y0=0;
  if (y1 > (int)f.ny) y1=f.ny;
  x0 += f.x;
  x1 += f.x;
  y0 += f.y;
  y1 += f.y;
  return true;
}

