#include <unistd.h>
#include "pdsdata/epics/EpicsXtcSettings.hh"

namespace Pds
{
    
namespace EpicsXtcSettings
{    
    const DetInfo detInfo( getpid(), Pds::DetInfo::EpicsArch, 0, DetInfo::NoDevice, 0);    
}

} 
