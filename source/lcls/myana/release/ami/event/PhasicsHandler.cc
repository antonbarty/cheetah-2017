#include "ami/event/PhasicsHandler.hh"
#include "pds/config/PhasicsConfigType.hh"
#include "pdsdata/camera/FrameFexConfigV1.hh"

using namespace Ami;

static std::list<Pds::TypeId::Type> config_type_list()
{
  std::list<Pds::TypeId::Type> types;
  types.push_back(Pds::TypeId::Id_PhasicsConfig);
  return types;
}

static Pds::Camera::FrameCoord roi(0,0);

static Pds::Camera::FrameFexConfigV1 _dummy(Pds::Camera::FrameFexConfigV1::FullFrame,
    1, Pds::Camera::FrameFexConfigV1::NoProcessing,
    roi, roi, 0, 0, 0);

PhasicsHandler::PhasicsHandler(const Pds::DetInfo& info) :
  FrameHandler(info, 
               config_type_list(),
               PhasicsConfigType::Width,
               PhasicsConfigType::Height)
{
}

void PhasicsHandler::_configure(const void* payload, const Pds::ClockTime& t)
{
  FrameHandler::_configure(&_dummy,t);
}

// For now this code assumes desc.ppxbin() == desc.ppybin() == 1
// just so I can more easily figure out what is going on
// (plus, it's true for our existing Phasics xtc files)

#include <math.h>

inline double& PhasicsHandler::_index(double a[], int x, int y) {
  return a[y * _npx + x];
}

// Rotate the intensity map
void PhasicsHandler::_rotate(double Ixy[], double Ixyr[], double phi) {
  double cphi = cos(phi);
  double sphi = sin(phi);

  bzero(Ixyr, _npx * _npy * sizeof(*Ixyr));
  int u, v;
  for (v = 0; v < _npy; v++) {
    for (u = 0; u < _npx; u++) {
      double x =  (u-_npx/2.0)*cphi + (v-_npy/2.0)*sphi + (_npx/2.0);
      double y = -(u-_npx/2.0)*sphi + (v-_npy/2.0)*cphi + (_npy/2.0);
      if (x < 1 || x > _npx-2 || y < 1 || y > _npy-2) {
        _index(Ixyr, u, v) = 0;
        continue;
      }
      int ix = (int) floor(x);
      int iy = (int) floor(y);
      double ddx = x - ix;
      double ddy = y - iy;
      double sxp, sxc, sxm;
      if (ddx < 0) {
        sxp = 0;
        sxc = 1 + ddx;
        sxm = -ddx;
      } else {
        sxp = ddx;
        sxc = 1 - ddx;
        sxm = 0;
      }
      double syp, syc, sym;
      if (ddy < 0) {
        syp = 0;
        syc = 1 + ddy;
        sym = -ddy;
      } else {
        syp = ddy;
        syc = 1 - ddy;
        sym = 0;
      }                    
      _index(Ixyr, u, v) = (sxp * syp * _index(Ixy, ix + 1, iy + 1) +
                            sxc * syp * _index(Ixy, ix + 0, iy + 1) +
                            sxm * syp * _index(Ixy, ix - 1, iy + 1) +
                            sxp * syc * _index(Ixy, ix + 1, iy + 0) +
                            sxc * syc * _index(Ixy, ix + 0, iy + 0) +
                            sxm * syc * _index(Ixy, ix - 1, iy + 0) +
                            sxp * sym * _index(Ixy, ix + 1, iy - 1) +
                            sxc * sym * _index(Ixy, ix + 0, iy - 1) +
                            sxm * sym * _index(Ixy, ix - 1, iy - 1));
    }
  }
}

template <class T>
void PhasicsHandler::_fill(const Pds::Camera::FrameV1& f, EntryImage& entry)
{
  const DescImage& desc = entry.desc();
  const T* d = reinterpret_cast<const T*>(f.data());
  double Ixy[_npx * _npy];
  for (unsigned y = 0; y < f.height(); y++) {
    for(unsigned x = 0; x < f.width(); x++) {
      _index(Ixy, x, y) = *d++;
    }
  }

  double Ixyr[_npx * _npy];
  double slope = 62.8 / 102.0;
  double phi = atan(slope);
  _rotate(Ixy, Ixyr, -phi);

  for (unsigned y = 0; y < f.height(); y++) {
    for(unsigned x = 0; x < f.width(); x++) {
      unsigned val = (unsigned) (_index(Ixyr, x, y) + 0.5);
      entry.addcontent(val, x, y);
    }
  }
  entry.info(f.offset(), EntryImage::Pedestal);
  entry.info(1, EntryImage::Normalization);
}

void PhasicsHandler::_event(const void* payload, const Pds::ClockTime& t)
{
  const Pds::Camera::FrameV1& f = *reinterpret_cast<const Pds::Camera::FrameV1*>(payload);

  // This is where we first start processing the FrameV1 data.
  printf("============================================================\n"); 
  printf("PhasicsHandler.cc: frame: width=%d height=%d depth=%d offset=%d, depth_bytes=%d, data_size=%d\n",
         f.width(),
         f.height(),
         f.depth(),
         f.offset(),
         f.depth_bytes(),
         f.data_size());
  printf("~~~ _entry->desc().nbinsx() = %d\n", _entry->desc().nbinsx());
  printf("~~~ _entry->desc().nbinsy() = %d\n", _entry->desc().nbinsy());
  printf("~~~ _entry->desc().ppxbin() = %d\n", _entry->desc().ppxbin());
  printf("~~~ _entry->desc().ppybin() = %d\n", _entry->desc().ppybin());
  printf("============================================================\n");

  _npx = f.width();
  _npy = f.height();

  if (!_entry) return;

  memset(_entry->contents(), 0, _entry->desc().nbinsx() * _entry->desc().nbinsy()* sizeof(unsigned));

  if (f.depth_bytes()==2)
    _fill<uint16_t>(f,*_entry);
  else
    _fill<uint8_t >(f,*_entry);

  _entry->valid(t);
}
