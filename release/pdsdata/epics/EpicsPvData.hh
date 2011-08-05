#ifndef EPICS_PV_DATA_H
#define EPICS_PV_DATA_H
#include "EpicsDbrTools.hh"

namespace Pds
{

class EpicsPvHeader
{
public:
    /** 
     * Layout of the EpicsPvHeader
     *
     * Name                | Length(Bytes)   |     Type     |    Description
     *---------------------+-----------------+--------------+------------------------------------
     * iPvId                      2              Int16        Pv Id
     * iDbrType                   2              Int16        Epics Data Type
     *                                                           Time Types: DB_TIME_STRING ~ DBR_TIME_DOUBLE
     *                                                           Ctrl Types: DB_CTRL_STRING ~ DBR_CTRL_DOUBLE
     * iNumElements               2              Int16        Size of Pv Array
     */
    short int   iPvId;
    short int   iDbrType;
    short int   iNumElements;         
    
    EpicsPvHeader( short int iPvId1, short int iDbrType1, short int iNumElements1 ) :
      iPvId(iPvId1), iDbrType(iDbrType1), iNumElements(iNumElements1)
    {}
    
    int printPv() const;

    static void* operator new(size_t size, char* p)     { return p; }
    // Disable ordinary (non-placement) new: only placement new and memory mapped objects are allowed
    // Disable placement delete: Not allow to delete memory mapped objects
    
private:   

    template <int iDbrType> 
    int printTimePvByDbrId() const;

    template <int iDbrType> 
    int printCtrlPvByDbrId() const;

    static const int _iSizeAllDbrTypes = EpicsDbrTools::iSizeAllDbrTypes;
    typedef int (EpicsPvHeader::*TPrintPvFuncPointer)() const;
    static const TPrintPvFuncPointer lfuncPrintPvFunctionTable[_iSizeAllDbrTypes];
    

    // Class usage control: Value semantics is disabled
    EpicsPvHeader( const EpicsPvHeader& );    
    EpicsPvHeader& operator=(const EpicsPvHeader& );    
};


class EpicsPvCtrlHeader : public EpicsPvHeader
{
private:     
    const static int _iMaxPvNameLength = EpicsDbrTools::iMaxPvNameLength;    
public:
    /** 
     * Layout of the EpicsPvCtrlHeader
     *
     *---------------------------------- EpicsPvHeader ---------------------------------------------
     *---------------------+-----------------+--------------+------------------------------------
     * Name                | Length(Bytes)   |     Type     |    Description
     *---------------------+-----------------+--------------+------------------------------------
     * iPvId                      2              Int16        Pv Id
     * iDbrType                   2              Int16        Epics Data Type
     *                                                           Ctrl Types: DB_CTRL_STRING ~ DBR_CTRL_DOUBLE
     * iNumElements               2              Int16        Size of Pv Array
     *
     *---------------------------------- EpicsPvCtrlHeader ---------------------------------------------
     *---------------------+-----------------+--------------+------------------------------------
     * Name                | Length(Bytes)   |     Type     |    Description
     *---------------------+-----------------+--------------+------------------------------------
     * sPvName              _iMaxPvNameLength   char []       Null-terminated string,
     */
    char        sPvName[_iMaxPvNameLength];
    
    EpicsPvCtrlHeader( short int iPvId1, short int iDbrType1, short int iNumElements1, const char sPvName1[] ) :
      EpicsPvHeader( iPvId1, iDbrType1, iNumElements1)
    {
        strncpy( sPvName, sPvName1, _iMaxPvNameLength );        
        if ( sPvName[_iMaxPvNameLength-1] != 0 )
	{
          printf("EpicsPvCtrlHeader::EpicsPvCtrlHeader(): Pv Name %s\n" 
            "  is too long for buffer size %d\n", sPvName1, _iMaxPvNameLength );
          sPvName[_iMaxPvNameLength-1] = 0;
	}
    }
};
    
/** 
 * Layout of the EpicsPvCtrl
 *
 *---------------------------------- EpicsPvHeader ---------------------------------------------
 *---------------------+-----------------+--------------+------------------------------------
 * Name                | Length(Bytes)   |     Type     |    Description
 *---------------------+-----------------+--------------+------------------------------------
 * iPvId                      2              Int16        Pv Id
 * iDbrType                   2              Int16        Epics Data Type
 *                                                           Ctrl Types: DB_CTRL_STRING ~ DBR_CTRL_DOUBLE
 * iNumElements               2              Int16        Size of Pv Array
 *
 *---------------------------------- EpicsPvCtrlHeader ---------------------------------------------
 *---------------------+-----------------+--------------+------------------------------------
 * Name                | Length(Bytes)   |     Type     |    Description
 *---------------------+-----------------+--------------+------------------------------------
 * sPvName              _iMaxPvNameLength   char []       Null-terminated string,
 *
 *-------- Begin of Epics structs (EpicsPvBase). Some structs may contain padding fields -------
 *---------------------------- EpicsPvCtrl< All > ----------------------------------------------
 * Name                | Length(Bytes)   |     Type     |    Description
 *---------------------+-----------------+--------------+------------------------------------
 * status                     2              Int16        Status (Enum State)
 *                                                         ref: epicsAlarmConditionStrings[Status]
 * severity                   2              Int16        Severity (Enum State)
 *                                                         ref: epicsAlarmSeverityStrings[Severity]
 *
 *--------------------- EpicsPvCtrl< DBR_FLOAT and DBR_DOUBLE > --------------------------------
 * Name                | Length(Bytes)   |     Type     |    Description
 *---------------------+-----------------+--------------+------------------------------------
 * precision                  2               Int16       Precision Digits
 *                                                          Only for Double and Float types
 *--------------------- EpicsPvCtrl< All except DBR_ENUM and DBR_STRING > ----------------------
 * Name                | Length(Bytes)   |     Type     |    Description
 *---------------------+-----------------+--------------+------------------------------------
 * units                MAX_UNITS_SIZE(8)   char []       Null-terminated string
 * upper_disp_limit     (Length and Type depend on Epics Data Type )  
 * lower_disp_limit     (Length and Type depend on Epics Data Type )  
 * upper_alarm_limit    (Length and Type depend on Epics Data Type )  
 * upper_warning_limit  (Length and Type depend on Epics Data Type )  
 * lower_warning_limit  (Length and Type depend on Epics Data Type )  
 * lower_alarm_limit    (Length and Type depend on Epics Data Type )  
 * upper_ctrl_limit     (Length and Type depend on Epics Data Type )  
 * lower_ctrl_limit     (Length and Type depend on Epics Data Type )  
 *
 *------------------------ EpicsPvCtrl< DBR_ENUM >----------------------------------------------
 * Name                | Length(Bytes)   |     Type     |    Description
 *---------------------+-----------------+--------------+------------------------------------
 * no_str                     2               Int16       Size of Enum State Array
 * strs[0]              MAX_ENUM_STRING_SIZE  char []     Null-terminated string
 * strs[1]              MAX_ENUM_STRING_SIZE  char []     Null-terminated string
 * strs[MAX_ENUM_STATES-1] ...
 *
 *------------------------ EpicsPvCtrl< DBR_STRING >--------------------------------------------
 *                     (No Control Value for String Type)
 *
 *---------------------------- EpicsPvCtrl< All > ----------------------------------------------
 * Name                | Length(Bytes)   |     Type     |    Description
 *---------------------+-----------------+--------------+------------------------------------
 * (opt) Padding Field  (Length and Type depend on Epics Data Type )  
 * value                (Length and Type depend on Epics Data Type )  
 *
 *--------------------- End of Epics structs (EpicsPvBase) -------------------------------------
 *
 * Value 2              (Length and Type depend on Epics Data Type )       
 * ...                  (Length and Type depend on Epics Data Type )       
 *
 *
 */
template <int iDbrType1, class EpicsPvBase = 
  typename EpicsDbrTools::DbrTypeTraits<iDbrType1>::TDbrCtrl
> 
class EpicsPvCtrl : 
  public EpicsPvCtrlHeader, public EpicsPvBase
{
public:
    enum { iDbrCtrlType = EpicsDbrTools::DbrTypeTraits<iDbrType1>::iDbrCtrlType };

    EpicsPvCtrl( short int iPvId1, short int iNumElements1, const char sPvName1[], void* pEpicsDataValue1, int* piSize = NULL ) :
      EpicsPvCtrlHeader(iPvId1, iDbrCtrlType, iNumElements1, sPvName1),
      EpicsPvBase( *(EpicsPvBase*) pEpicsDataValue1 )
    {
        if ( iNumElements > 1 )
            memcpy( this + 1, (EpicsPvBase*) pEpicsDataValue1 + 1, sizeof(EpicsPvBase::value) * (iNumElements-1) );
            
        if ( piSize != NULL )
            *piSize = getSize();
    }
    
    typedef typename EpicsDbrTools::DbrTypeTraits<iDbrType1>::TDbrOrg TDbrOrg;    
    
    int printPv() const;        
        
private:
    int getSize() const
    {
        return sizeof(*this) + sizeof(EpicsPvBase::value) * (iNumElements-1);        
    }

    // Class usage control:
    //   Inherited from EpicsPvHeader:
    //     Value semantics is disabled
    //     Disable ordinary (non-placement) new: only placement new and memory mapped objects are allowed
    //     Disable placement delete: Not allow to delete memory mapped objects
};
 

/** 
 * Layout of the EpicsPvTime
 *
 *---------------------------------- EpicsPvHeader ---------------------------------------------
 *---------------------+-----------------+--------------+------------------------------------
 * Name                | Length(Bytes)   |     Type     |    Description
 *---------------------+-----------------+--------------+------------------------------------
 * iPvId                      2              Int16        Pv Id
 * iDbrType                   2              Int16        Epics Data Type
 *                                                           Ctrl Types: DB_CTRL_STRING ~ DBR_CTRL_DOUBLE
 * iNumElements               2              Int16        Size of Pv Array
 *
 *-------- Begin of Epics structs (EpicsPvBase). Some structs may contain padding fields -------
 *---------------------------- EpicsPvCtrl< All > ----------------------------------------------
 * Name                | Length(Bytes)   |     Type     |    Description
 *---------------------+-----------------+--------------+------------------------------------
 * status                     2              Int16        Status (Enum State)
 *                                                         ref: epicsAlarmConditionStrings[Status]
 * severity                   2              Int16        Severity (Enum State)
 *                                                         ref: epicsAlarmSeverityStrings[Severity]
 * stamp                      8           epicsDataStamp  Time Stamp. Null-terminated string.
 * (opt) Padding Field  (Length and Type depend on Epics Data Type )  
 * value                (Length and Type depend on Epics Data Type )  
 *
 *------------------ End of Epics structs (EpicsPvBase) ----------------------------------------
 *
 * value 2              (Length and Type depend on Epics Data Type )       
 * ...                  (Length and Type depend on Epics Data Type )       
 *
 */
template <int iDbrType1, class EpicsPvBase = 
  typename EpicsDbrTools::DbrTypeTraits<iDbrType1>::TDbrTime
> 
class EpicsPvTime : 
  public EpicsPvHeader, public EpicsPvBase
{
public:
    enum { iDbrTimeType = EpicsDbrTools::DbrTypeTraits<iDbrType1>::iDbrTimeType };

    EpicsPvTime( short int iPvId1, int iNumElements1, void* pEpicsDataValue1, int* piSize = NULL ) :
      EpicsPvHeader( iPvId1, iDbrTimeType, iNumElements1 ),
      EpicsPvBase( *(EpicsPvBase*) pEpicsDataValue1 )
    {
        if ( iNumElements > 1 )
            memcpy( this + 1, (EpicsPvBase*) pEpicsDataValue1 + 1, sizeof(EpicsPvBase::value) * (iNumElements-1) );
            
        if ( piSize != NULL )
            *piSize = getSize();
    }
    
    typedef typename EpicsDbrTools::DbrTypeTraits<iDbrType1>::TDbrOrg TDbrOrg;    
    
    int printPv() const;        
        
private:
    int getSize() const
    {
        return sizeof(*this) + sizeof(EpicsPvBase::value) * (iNumElements-1);        
    }

    // Class usage control:
    //   Inherited from EpicsPvHeader:
    //     Value semantics is disabled
    //     Disable ordinary (non-placement) new: only placement new and memory mapped objects are allowed
    //     Disable placement delete: Not allow to delete memory mapped objects
};

// template function definitions
template <int iDbrType1> 
int EpicsPvHeader::printCtrlPvByDbrId() const
{
    return ((const EpicsPvCtrl<iDbrType1>*) this) -> printPv();
}

// template function definitions
template <int iDbrType1> 
int EpicsPvHeader::printTimePvByDbrId() const
{
    return ((const EpicsPvTime<iDbrType1>*) this) -> printPv();
}

template <int iDbrType1, class EpicsPvBase> 
int EpicsPvCtrl<iDbrType1, EpicsPvBase> ::printPv() const
{
    printf( "\n> PV (Ctrl) Id %d\n", this->iPvId ); 
    printf( "Name: %s\n", this->sPvName ); 
    printf( "Type: %s\n", Epics::dbr_text[this->iDbrType] ); 
    if ( this->iNumElements > 1 ) printf("Length: %d\n", this->iNumElements);
    
    printf("Status: %s\n", Epics::epicsAlarmConditionStrings[this->status] );
    printf("Severity: %s\n", Epics::epicsAlarmSeverityStrings[this->severity] );    
        
    EpicsDbrTools::printCtrlFields( *static_cast<const EpicsPvBase*>(this) );
    
    printf( "Value: " );
    const TDbrOrg* pValue = &this->value;
    for ( int iElement = 0; iElement < this->iNumElements; iElement++, pValue++ )
    {
        EpicsDbrTools::printValue(pValue);
        if ( iElement < this->iNumElements-1 ) printf( ", " );
    }
    printf( "\n" );         

    return 0;
}

template <int iDbrType1, class EpicsPvBase> 
int EpicsPvTime<iDbrType1, EpicsPvBase> ::printPv() const
{
    printf( "\n> PV (Time) Id %d\n", this->iPvId); 
    printf( "Type: %s\n", Epics::dbr_text[this->iDbrType] ); 
    if ( this->iNumElements > 1 ) printf("Length: %d\n", this->iNumElements);
    
    printf("Status: %s\n", Epics::epicsAlarmConditionStrings[this->status] );
    printf("Severity: %s\n", Epics::epicsAlarmSeverityStrings[this->severity] );    
    
    static const char timeFormatStr[40] = "%04Y-%02m-%02d %02H:%02M:%02S"; /* Time format string */    
    char sTimeText[40];
    
    struct tm tmTimeStamp;
    localtime_r( (const time_t*) (void*) &this->stamp.secPastEpoch, &tmTimeStamp );
    tmTimeStamp.tm_year += 20; // Epics Epoch starts from 1990, whereas linux time.h Epoch starts from 1970    
    
    strftime(sTimeText, sizeof(sTimeText), timeFormatStr, &tmTimeStamp );

    printf( "TimeStamp: %s.%09u\n", sTimeText, (unsigned int) this->stamp.nsec );
   
    printf( "Value: " );
    const TDbrOrg* pValue = &this->value;
    for ( int iElement = 0; iElement < this->iNumElements; iElement++, pValue++ )
    {
        EpicsDbrTools::printValue(pValue);
        if ( iElement < this->iNumElements-1 ) printf( ", " );
    }
    printf( "\n" );     
    
    return 0;
}

} // namespace Pds    
    
#endif
