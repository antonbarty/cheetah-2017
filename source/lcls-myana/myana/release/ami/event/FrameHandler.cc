#include "FrameHandler.hh"

#include "ami/data/EntryImage.hh"
#include "ami/data/ChannelID.hh"
#include "pdsdata/camera/FrameV1.hh"
#include "pdsdata/camera/FrameFexConfigV1.hh"
#include "pdsdata/opal1k/ConfigV1.hh"

#include <string.h>

using namespace Ami;

FrameHandler::FrameHandler(const Pds::DetInfo& info,
			   unsigned defColumns,
			   unsigned defRows) : 
  EventHandler(info, Pds::TypeId::Id_Frame, Pds::TypeId::Id_FrameFexConfig),
  _entry(0),
  _defColumns(defColumns),
  _defRows   (defRows)
{
}

FrameHandler::FrameHandler(const Pds::DetInfo& info,
			   const std::list<Pds::TypeId::Type>& config_types,
			   unsigned defColumns,
			   unsigned defRows) : 
  EventHandler(info, Pds::TypeId::Id_Frame, config_types),
  _entry(0),
  _defColumns(defColumns),
  _defRows   (defRows)
{
}

FrameHandler::FrameHandler(const Pds::DetInfo& info, const EntryImage* entry) : 
  EventHandler(info, Pds::TypeId::Id_Frame, Pds::TypeId::Id_FrameFexConfig),
  _entry(entry ? new EntryImage(entry->desc()) : 0)
{
}

FrameHandler::~FrameHandler()
{
  if (_entry)
    delete _entry;
}

unsigned FrameHandler::nentries() const { return _entry ? 1 : 0; }

const Entry* FrameHandler::entry(unsigned i) const { return i==0 ? _entry : 0; }

void FrameHandler::reset() { _entry = 0; }

void FrameHandler::_configure(const void* payload, const Pds::ClockTime& t)
{
  const Pds::Camera::FrameFexConfigV1& c = *reinterpret_cast<const Pds::Camera::FrameFexConfigV1*>(payload);
  const Pds::DetInfo& det = static_cast<const Pds::DetInfo&>(info());
  unsigned columns,rows;
  if (c.forwarding() == Pds::Camera::FrameFexConfigV1::FullFrame) {
    columns = _defColumns;
    rows    = _defRows;
  }
  else {
    columns = c.roiEnd().column-c.roiBegin().column;
    rows    = c.roiEnd().row   -c.roiBegin().row   ;
  }
  unsigned pixels  = (columns > rows) ? columns : rows;
  unsigned ppb     = _full_resolution() ? 1 : (pixels-1)/640 + 1;
  columns = (columns+ppb-1)/ppb;
  rows    = (rows   +ppb-1)/ppb;
  DescImage desc(det, (unsigned)0, ChannelID::name(det),
		 columns, rows, ppb, ppb);

  if (_entry) 
    delete _entry;
  _entry = new EntryImage(desc);
  _entry->invalid();
}

void FrameHandler::_calibrate(const void* payload, const Pds::ClockTime& t) {}

template <class T>
void _fill(const Pds::Camera::FrameV1& f, EntryImage& entry)
{
  const DescImage& desc = entry.desc();

  const T* d = reinterpret_cast<const T*>(f.data());
  for(unsigned j=0; j<f.height(); j++) {
    unsigned iy = j/desc.ppybin();
    switch(desc.ppxbin()) {
    case 1:
      for(unsigned k=0; k<f.width(); k++, d++)
	entry.addcontent(*d, k, iy);
      break;
    case 2:
      for(unsigned k=0; k<f.width(); k++, d++)
	entry.addcontent(*d, k>>1, iy);
      break;
    case 4:
      for(unsigned k=0; k<f.width(); k++, d++)
	entry.addcontent(*d, k>>2, iy);
      break;
    default:
      for(unsigned k=0; k<f.width(); k++, d++)
	entry.addcontent(*d, k/desc.ppxbin(), iy);
      break;
    }
  }

  entry.info(f.offset()*desc.ppxbin()*desc.ppybin(),EntryImage::Pedestal);
  entry.info(1,EntryImage::Normalization);
}

void FrameHandler::_event    (const void* payload, const Pds::ClockTime& t)
{
  const Pds::Camera::FrameV1& f = *reinterpret_cast<const Pds::Camera::FrameV1*>(payload);
  if (!_entry) return;

  memset(_entry->contents(),0,_entry->desc().nbinsx()*_entry->desc().nbinsy()*sizeof(unsigned));

  if (f.depth_bytes()==2)
    _fill<uint16_t>(f,*_entry);
  else
    _fill<uint8_t >(f,*_entry);

  _entry->valid(t);
}

void FrameHandler::_damaged() { _entry->invalid(); }
