#include "FccdHandler.hh"

#include "ami/data/EntryImage.hh"
#include "ami/data/ChannelID.hh"
#include "pdsdata/camera/FrameV1.hh"

#include <string.h>

using namespace Ami;

FccdHandler::FccdHandler(const Pds::DetInfo& info) : 
  EventHandler(info, Pds::TypeId::Id_Frame, Pds::TypeId::Id_FccdConfig),
  _entry(0)
{
}

FccdHandler::FccdHandler(const Pds::DetInfo& info, const EntryImage* entry) : 
  EventHandler(info, Pds::TypeId::Id_Frame, Pds::TypeId::Id_FccdConfig),
  _entry(entry ? new EntryImage(entry->desc()) : 0)
{
}

FccdHandler::~FccdHandler()
{
}

unsigned FccdHandler::nentries() const { return _entry ? 1 : 0; }

const Entry* FccdHandler::entry(unsigned i) const { return i==0 ? _entry : 0; }

void FccdHandler::reset() { _entry = 0; }

void FccdHandler::_configure(const void* payload, const Pds::ClockTime& t)
{
  const Pds::FCCD::FccdConfigV2& c = *reinterpret_cast<const Pds::FCCD::FccdConfigV2*>(payload);
  const Pds::DetInfo& det = static_cast<const Pds::DetInfo&>(info());
  DescImage desc(det, (unsigned)0, ChannelID::name(det),
                  c.trimmedWidth(),
                  c.trimmedHeight()); // FCCD image is 480 x 480 after removing dark pixels
  _entry = new EntryImage(desc);
}

void FccdHandler::_calibrate(const void* payload, const Pds::ClockTime& t) {}

void FccdHandler::_event    (const void* payload, const Pds::ClockTime& t)
{
  const Pds::Camera::FrameV1& f = *reinterpret_cast<const Pds::Camera::FrameV1*>(payload);
  if (!_entry) return;

  memset(_entry->contents(),0,_entry->desc().nbinsx()*_entry->desc().nbinsy()*sizeof(unsigned));
  const uint16_t* d = reinterpret_cast<const uint16_t*>(f.data());

  //  Height:
  //    500 rows   = 6 + 240 * 7 + 240 + 7
  //         Dark A: 6   Rows 0-5
  //       Data Top: 240 Rows 6-245
  //         Dark B: 7   Rows 246-252
  //    Data Bottom: 240 Rows 253-492
  //         Dark C: 7   Rows 493-249
  // 
  //  Width (in 16-bit pixels):
  //    576 pixels = 12 * 48 outputs
  //            Top: (10 image pixels followed by 2 info pixels) * 48 outputs
  //         Bottom: (2 info pixels followed by 10 image pixels) * 48 outputs

  unsigned i, j, k;

  // 6 top info rows skipped (Dark A)
  d += (6 * f.width() / 2);    // f.width() is in 8 bit pixels

  // top half of image -- 240 rows per half
  for (i = 6; i < 246; i++) {
    // 48 outputs
    for (j = 0; j < 48; j++) {
      // 10 image pixels per output
      for (k = 0; k < 10; k++) {
        _entry->addcontent(*d, (j*10)+k, i-6);  // 6 rows skipped above
        d++;
      }
      // 2 info pixels per output (skipped)
      d += 2;
    }
  }
  // 7 middle info rows skipped (Dark B)
  d += (7 * f.width() / 2);    // f.width() is in 8 bit pixels

  // bottom half of image -- 240 rows per half
  for (i = 253; i < 493; i++) {
    // 48 outputs
    for (j = 0; j < 48; j++) {
      // 2 info pixels per output (skipped)
      d += 2;
      // 10 image pixels per output
      for (k = 0; k < 10; k++) {
        _entry->addcontent(*d, (j*10)+k, i-13); // 13 rows skipped above
        d++;
      }
    }
  }

  _entry->info(f.offset(),EntryImage::Pedestal);
  _entry->info(1,EntryImage::Normalization);
  _entry->valid(t);
}

void FccdHandler::_damaged() { _entry->invalid(); }
