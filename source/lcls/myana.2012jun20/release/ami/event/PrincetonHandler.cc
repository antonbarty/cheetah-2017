#include "PrincetonHandler.hh"

#include "ami/data/EntryImage.hh"
#include "ami/data/ChannelID.hh"
#include "pdsdata/princeton/ConfigV1.hh"
#include "pdsdata/princeton/ConfigV2.hh"
#include "pdsdata/princeton/FrameV1.hh"
#include "pdsdata/princeton/InfoV1.hh"

#include <string.h>

static inline unsigned height(const Pds::Princeton::ConfigV3& c)
{
  return (c.height() + c.binY() - 1)/c.binY();
}

static unsigned width(const Pds::Princeton::ConfigV3& c)
{
  return (c.width() + c.binX() - 1)/c.binX();
}

using namespace Ami;

static std::list<Pds::TypeId::Type> data_type_list()
{
  std::list<Pds::TypeId::Type> types;
  types.push_back(Pds::TypeId::Id_PrincetonFrame);
  types.push_back(Pds::TypeId::Id_PrincetonInfo);
  return types;
}

PrincetonHandler::PrincetonHandler(const Pds::DetInfo& info, FeatureCache& cache) : 
  EventHandler(info, data_type_list(), Pds::TypeId::Id_PrincetonConfig),
  _cache(cache),
  _iCacheIndexTemperature(-1),
  _entry(0)
{
}

//PrincetonHandler::PrincetonHandler(const Pds::DetInfo& info, const EntryImage* entry) : 
//  EventHandler(info, Pds::TypeId::Id_Frame, Pds::TypeId::Id_PrincetonConfig),
//  _entry(entry ? new EntryImage(entry->desc()) : 0)
//{
//}

PrincetonHandler::~PrincetonHandler()
{
}

unsigned PrincetonHandler::nentries() const { return _entry ? 1 : 0; }

const Entry* PrincetonHandler::entry(unsigned i) const { return i==0 ? _entry : 0; }

void PrincetonHandler::reset() { _entry = 0; }

void PrincetonHandler::_configure(Pds::TypeId type,const void* payload, const Pds::ClockTime& t)
{  
  if (type.version() == 3)
    _config = *reinterpret_cast<const Pds::Princeton::ConfigV3*>(payload);
  if (type.version() == 2)
    new (&_config) Pds::Princeton::ConfigV3(*reinterpret_cast<const Pds::Princeton::ConfigV2*>(payload));
  else if (type.version() == 1)
    new (&_config) Pds::Princeton::ConfigV2(*reinterpret_cast<const Pds::Princeton::ConfigV1*>(payload));
  else
    printf("PrincetonHandler::_configure(): Unsupported Princeton Version %d\n", type.version());
  
  unsigned columns = width (_config);
  unsigned rows    = height(_config);
  unsigned pixels  = (columns > rows) ? columns : rows;
  unsigned ppb     = _full_resolution() ? 1 : (pixels-1)/640 + 1;
  columns = (columns+ppb-1)/ppb;
  rows    = (rows   +ppb-1)/ppb;
  const Pds::DetInfo& det = static_cast<const Pds::DetInfo&>(info());
  DescImage desc(det, (unsigned)0, ChannelID::name(det),
     columns, rows, ppb, ppb);
  _entry  = new EntryImage(desc);
    
  /*
   * Setup temperature variable
   */
  char sTemperatureVar[64];  
  sprintf(sTemperatureVar, "Princeton-%d-T", det.devId());
  _iCacheIndexTemperature = _cache.add(sTemperatureVar);
}

/*
 * This function will never be called. The above _configure() replaces this one.
 */
void PrincetonHandler::_configure(const void* payload, const Pds::ClockTime& t)
{
  abort();
}

void PrincetonHandler::_calibrate(const void* payload, const Pds::ClockTime& t) {}

void PrincetonHandler::_event(Pds::TypeId type, const void* payload, const Pds::ClockTime& t)
{
  if (type.id() == Pds::TypeId::Id_PrincetonFrame)
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
  else if (type.id() == Pds::TypeId::Id_PrincetonInfo)
  {
    const Pds::Princeton::InfoV1& info1 = *reinterpret_cast<const Pds::Princeton::InfoV1*>(payload);
    if (_iCacheIndexTemperature != -1)
      _cache.cache(_iCacheIndexTemperature, info1.temperature());
  }
}

/*
 * This function will never be called. The above _event() replaces this one.
 */
void PrincetonHandler::_event(const void* payload, const Pds::ClockTime& t)
{
  abort();
}

void PrincetonHandler::_damaged() { _entry->invalid(); }
