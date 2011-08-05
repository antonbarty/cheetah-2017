#include "ami/data/EntryImage.hh"

static const unsigned DefaultNbins = 1;
static const float DefaultLo = 0;
static const float DefaultUp = 1;
static const unsigned DefaultPedestal = 128;

using namespace Ami;

#define SIZE(nx,ny) (nx*ny+InfoSize)

EntryImage::~EntryImage() {}

EntryImage::EntryImage(const Pds::DetInfo& info, unsigned channel, const char* name) :
  _desc(info, channel, name, DefaultNbins, DefaultNbins)
{
  build(DefaultNbins, DefaultNbins);
}

EntryImage::EntryImage(const DescImage& desc) :
  _desc(desc)
{
  build(_desc.nbinsx(), _desc.nbinsy());
}

void EntryImage::params(unsigned nbinsx,
			   unsigned nbinsy,
			   int ppxbin,
			   int ppybin)
{
  _desc.params(nbinsx, nbinsy, ppxbin, ppybin);
  build(nbinsx, nbinsy);
}

void EntryImage::params(const DescImage& desc)
{
  _desc = desc;
  build(_desc.nbinsx(), _desc.nbinsy());
}

void EntryImage::build(unsigned nbinsx, unsigned nbinsy)
{
  _y = static_cast<unsigned*>(allocate(sizeof(unsigned)*SIZE(nbinsx,nbinsy)));
  info(DefaultPedestal, EntryImage::Pedestal);
}

const DescImage& EntryImage::desc() const {return _desc;}
DescImage& EntryImage::desc() {return _desc;}

void EntryImage::setto(const EntryImage& entry) 
{
  unsigned* dst = _y;
  const unsigned* end = dst+SIZE(_desc.nbinsx(),_desc.nbinsy());
  const unsigned* src = entry._y;
  do {
    *dst++ = *src++;
  } while (dst < end);
  valid(entry.time());
}

void EntryImage::setto(const EntryImage& curr, 
			  const EntryImage& prev)
{
  unsigned* dst = _y;
  const unsigned* end = dst+SIZE(_desc.nbinsx(),_desc.nbinsy());
  const unsigned* srccurr = curr._y;
  const unsigned* srcprev = prev._y;
  do {
    *dst++ = *srccurr++ - *srcprev++;
  } while (dst < end);
  valid(curr.time());
}

void EntryImage::add  (const EntryImage& entry) 
{
  unsigned* dst = _y;
  const unsigned* end = dst+SIZE(_desc.nbinsx(),_desc.nbinsy());
  const unsigned* src = entry._y;
  do {
    *dst++ += *src++;
  } while (dst < end);
}

