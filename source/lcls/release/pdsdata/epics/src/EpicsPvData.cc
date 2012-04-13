#include "pdsdata/epics/EpicsPvData.hh"

namespace Pds
{

const EpicsPvHeader::TPrintPvFuncPointer EpicsPvHeader::lfuncPrintPvFunctionTable[] =
{
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, // No support for fundamental types
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, // No support for status (STS) types
    /* Time Types */
    &EpicsPvHeader::printTimePvByDbrId<DBR_STRING>, &EpicsPvHeader::printTimePvByDbrId<DBR_SHORT>,
    &EpicsPvHeader::printTimePvByDbrId<DBR_FLOAT>,  &EpicsPvHeader::printTimePvByDbrId<DBR_ENUM>,
    &EpicsPvHeader::printTimePvByDbrId<DBR_CHAR>,   &EpicsPvHeader::printTimePvByDbrId<DBR_LONG>,
    &EpicsPvHeader::printTimePvByDbrId<DBR_DOUBLE>,    
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, // No support for graphic (GR) types
    /* Ctrl Types */
    &EpicsPvHeader::printCtrlPvByDbrId<DBR_STRING>, &EpicsPvHeader::printCtrlPvByDbrId<DBR_SHORT>,
    &EpicsPvHeader::printCtrlPvByDbrId<DBR_FLOAT>,  &EpicsPvHeader::printCtrlPvByDbrId<DBR_ENUM>,
    &EpicsPvHeader::printCtrlPvByDbrId<DBR_CHAR>,   &EpicsPvHeader::printCtrlPvByDbrId<DBR_LONG>,
    &EpicsPvHeader::printCtrlPvByDbrId<DBR_DOUBLE>
};

    
int EpicsPvHeader::printPv() const
{    
    if ( dbr_type_is_CTRL(iDbrType ) || dbr_type_is_TIME(iDbrType) )
        ( this ->* lfuncPrintPvFunctionTable[iDbrType] ) ();
    else
    {
        printf( "EpicsPvHeader::printPv(): Invalid Pv Type: %d\n", iDbrType );
        return 1;
    }        
    
    return 0;
}

} // namespace Pds    
