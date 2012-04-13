#include "PrincetonHandler.hh"

#include "ami/data/EntryImage.hh"
#include "ami/data/ChannelID.hh"
#include "pdsdata/princeton/FrameV1.hh"

#include <string.h>

static inline unsigned height(const Pds::Princeton::ConfigV1& c)
{
  return (c.height() + c.binY() - 1)/c.binY();
}

static unsigned width(const Pds::Princeton::ConfigV1& c)
{
  return (c.width() + c.binX() - 1)/c.binX();
}

using namespace Ami;

PrincetonHandler::PrincetonHandler(const Pds::DetInfo& info) : 
  EventHandler(info, Pds::TypeId::Id_PrincetonFrame, Pds::TypeId::Id_PrincetonConfig),
  _entry(0)
{
}

PrincetonHandler::PrincetonHandler(const Pds::DetInfo& info, const EntryImage* entry) : 
  EventHandler(info, Pds::TypeId::Id_Frame, Pds::TypeId::Id_PrincetonConfig),
  _entry(entry ? new EntryImage(entry->desc()) : 0)
{
}

PrincetonHandler::~PrincetonHandler()
{
}

unsigned PrincetonHandler::nentries() const { return _entry ? 1 : 0; }

const Entry* PrincetonHandler::entry(unsigned i) const { return i==0 ? _entry : 0; }

void PrincetonHandler::reset() { _entry = 0; }

void PrincetonHandler::_configure(const void* payload, const Pds::ClockTime& t)
{
  const Pds::Princeton::ConfigV1& c = *reinterpret_cast<const Pds::Princeton::ConfigV1*>(payload);
  unsigned columns = width (c);
  unsigned rows    = height(c);
  unsigned pixels  = (columns > rows) ? columns : rows;
  unsigned ppb     = (pixels-1)/640 + 1;
  columns = (columns+ppb-1)/ppb;
  rows    = (rows   +ppb-1)/ppb;
  const Pds::DetInfo& det = static_cast<const Pds::DetInfo&>(info());
  DescImage desc(det, (unsigned)0, ChannelID::name(det),
		 columns, rows, ppb, ppb);
  _config = c;
  _entry  = new EntryImage(desc);
}

void PrincetonHandler::_calibrate(const void* payload, const Pds::ClockTime& t) {}

void PrincetonHandler::_event    (const void* payload, const Pds::ClockTime& t)
{
  const Pds::Princeton::FrameV1& f = *reinterpret_cast<const Pds::Princeton::FrameV1*>(payload);
  if (!_entry) return;

  const DescImage& desc = _entry->desc();
  unsigned ppbx = desc.ppxbin();
  unsigned ppby = desc.ppybin();
  memset(_entry->contents(),0,desc.nbinsx()*desc.nbinsy()*sizeof(unsigned));
  const uint16_t* d = reinterpret_cast<const uint16_t*>(f.data());
  for(unsigned j=0; j<height(_config); j++)
    for(unsigned k=0; k<width(_config); k++, d++)
      _entry->addcontent(*d, k/ppbx, j/ppby);

  //  _entry->info(f.offset()*ppbx*ppby,EntryImage::Pedestal);
  _entry->info(0,EntryImage::Pedestal);
  _entry->info(1,EntryImage::Normalization);
  _entry->valid(t);
}

void PrincetonHandler::_damaged() { _entry->invalid(); }
