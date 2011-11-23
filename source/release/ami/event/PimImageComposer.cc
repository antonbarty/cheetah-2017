#include "PimImageComposer.hh"

#include "ami/event/EventHandler.hh"
#include "ami/data/DescImage.hh"
#include "ami/data/EntryImage.hh"
#include "pdsdata/lusi/PimImageConfigV1.hh"

using namespace Ami;

PimImageComposer::PimImageComposer(const Pds::Src& info,
				   const void* payload) :
  Composer(info),
  _config (*reinterpret_cast<const Pds::Lusi::PimImageConfigV1*>(payload))
{
}

void PimImageComposer::compose(EventHandler& h)
{
  EntryImage* image = static_cast<EntryImage*>(const_cast<Entry*>(h.entry(0)));
  image->desc().set_scale(_config.xscale, _config.yscale);
}
