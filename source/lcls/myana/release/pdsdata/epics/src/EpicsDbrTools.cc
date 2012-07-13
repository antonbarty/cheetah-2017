#include <stdio.h>
#include "pdsdata/epics/EpicsDbrTools.hh"

namespace Pds
{
   
namespace EpicsDbrTools    
{

const char* sDbrPrintfFormat[] =
{
    "%s",   // DBR_STRING   returns a NULL terminated string
    "%d",   // DBR_SHORT  &&  DBR_INT      returns an unsigned short
    "%f",   // DBR_FLOAT    returns an IEEE floating point value
    "%d",   // DBR_ENUM returns an unsigned short which is the enum item
    "%d",   // DBR_CHAR    returns an unsigned char
    "%ld",  // DBR_LONG    returns an unsigned long
    "%lf"   // DBR_DOUBLE  returns a double precision floating point number
};

} // namespace EpicsDbrTools


namespace Epics
{
/*
 * Imported from Epics header file: ${EPICS_PROJECT_DIR}/base/current/include/alarm.h
 *  
 */
const char *epicsAlarmSeverityStrings [ALARM_NSEV] = 
{
    "NO_ALARM",
    "MINOR",
    "MAJOR",
    "INVALID",
};

const char *epicsAlarmConditionStrings [ALARM_NSTATUS] = 
{
    "NO_ALARM",
    "READ",
    "WRITE",
    "HIHI",
    "HIGH",
    "LOLO",
    "LOW",
    "STATE",
    "COS",
    "COMM",
    "TIMEOUT",
    "HWLIMIT",
    "CALC",
    "SCAN",
    "LINK",
    "SOFT",
    "BAD_SUB",
    "UDF",
    "DISABLE",
    "SIMM",
    "READ_ACCESS",
    "WRITE_ACCESS",
};    


/*
 * Imported from Epics source file: ${EPICS_PROJECT_DIR}/base/current/src/ca/access.cpp
 *  
 */
const char *dbr_text[] = 
{
    "DBR_STRING",
    "DBR_SHORT",
    "DBR_FLOAT",
    "DBR_ENUM",
    "DBR_CHAR",
    "DBR_LONG",
    "DBR_DOUBLE",
    "DBR_STS_STRING",
    "DBR_STS_SHORT",
    "DBR_STS_FLOAT",
    "DBR_STS_ENUM",
    "DBR_STS_CHAR",
    "DBR_STS_LONG",
    "DBR_STS_DOUBLE",
    "DBR_TIME_STRING",
    "DBR_TIME_SHORT",
    "DBR_TIME_FLOAT",
    "DBR_TIME_ENUM",
    "DBR_TIME_CHAR",
    "DBR_TIME_LONG",
    "DBR_TIME_DOUBLE",
    "DBR_GR_STRING",
    "DBR_GR_SHORT",
    "DBR_GR_FLOAT",
    "DBR_GR_ENUM",
    "DBR_GR_CHAR",
    "DBR_GR_LONG",
    "DBR_GR_DOUBLE",
    "DBR_CTRL_STRING",
    "DBR_CTRL_SHORT",
    "DBR_CTRL_FLOAT",
    "DBR_CTRL_ENUM",
    "DBR_CTRL_CHAR",
    "DBR_CTRL_LONG",
    "DBR_CTRL_DOUBLE",
};

} // namespace Epics
    
} // namespace Pds

