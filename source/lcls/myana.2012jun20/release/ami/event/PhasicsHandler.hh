#ifndef Ami_PhasicsHandler_hh
#define Ami_PhasicsHandler_hh

#include "ami/event/FrameHandler.hh"












#include "ami/data/EntryImage.hh"
#include "ami/data/ChannelID.hh"
#include "pdsdata/camera/FrameV1.hh"
#include "pdsdata/camera/FrameFexConfigV1.hh"
#include "pdsdata/opal1k/ConfigV1.hh"

#include <string.h>









namespace Ami {
  class PhasicsHandler : public FrameHandler {
  public:
    PhasicsHandler(const Pds::DetInfo& info);
  protected:
    void _configure(const void*, const Pds::ClockTime&);
    void _event(const void* payload, const Pds::ClockTime& t);
  private:
    int _npx;
    int _npy;
    double& _index(double a[], int x, int y);
    void _rotate(double Ixy[], double Ixyr[], double phi);
    template <class T> void _fill(const Pds::Camera::FrameV1& f, EntryImage& entry);
  };
};

#endif
