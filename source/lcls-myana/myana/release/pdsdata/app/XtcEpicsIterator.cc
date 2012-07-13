#include <stdio.h>
#include <string>
#include "XtcEpicsIterator.hh"

namespace Pds
{

using std::string;
const Src XtcEpicsIterator::srcLevel(Level::Segment); // For direct file recording

int XtcEpicsIterator::process(Xtc* xtc) 
{    
    printf( string( _iDepth*2, ' ' ).c_str() );
    
    
    Level::Type level = xtc->src.level();    
    printf("%s level: ",Level::name(level));
    
    
    if (level != srcLevel.level()) 
    {
        printf("XtcEpicsIterator::level (%s) is not correct (should be %s)\n", 
          Level::name( level ), Level::name( srcLevel.level() ) );
        return 0; // return Zero to stop recursive processing
    }
    
    int iVersion = (int) xtc->contains.version();
    TypeId::Type xtcTypeId = xtc->contains.id();
    switch ( xtcTypeId ) 
    {
    case ( TypeId::Id_Xtc ) : 
      {
        XtcEpicsIterator iter( xtc, _iDepth+1 );
        iter.iterate();
        break;
      }
    case ( typeIdXtc ) :
      {
        if ( iVersion != iXtcVersion ) 
        {
            printf( "Xtc TypdId version (%d) is not compatible with reader supported version (%d)", iVersion, iXtcVersion );
            break;
        }
        EpicsPvHeader* pEpicsPvData = (EpicsPvHeader*) ( xtc->payload() );
        pEpicsPvData->printPv();
        printf( "\n" );
      }
      break;
    default :
        printf("XtcEpicsIterator::type id %s is not correct (should be %s)\n", TypeId::name( xtcTypeId ), TypeId::name( typeIdXtc ) );
        return 0; // return Zero to stop recursive processing
    }

    return 1; // return NonZero value to let upper level Xtc continue the processing
}
  
} // namespace Pds
