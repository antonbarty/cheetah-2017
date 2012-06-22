#ifndef EPICS_DBR_TOOLS_H
#define EPICS_DBR_TOOLS_H

#include <string>
#include <stdint.h>

#if defined(INCLcadefh)
#define EPICS_HEADERS_INCLUDED
#endif

namespace Pds
{

/*
 * Imported from Epics library
 */        
namespace Epics
{

#ifndef EPICS_HEADERS_INCLUDED    
    
/*
 * Imported from Epics header file: ${EPICS_PROJECT_DIR}/base/current/include/alarm.h
 */    
#define ALARM_NSEV      4
#define ALARM_NSTATUS   22

/*
 * Imported from Epics header file: ${EPICS_PROJECT_DIR}/base/current/include/epicsTypes.h
 */    

#if __STDC_VERSION__ >= 199901L
    typedef int8_t          epicsInt8;
    typedef uint8_t         epicsUInt8;
    typedef int16_t         epicsInt16;
    typedef uint16_t        epicsUInt16;
    typedef epicsUInt16     epicsEnum16;
    typedef int32_t         epicsInt32;
    typedef uint32_t        epicsUInt32;
    typedef int64_t         epicsInt64;
    typedef uint64_t        epicsUInt64;
#else
    typedef char            epicsInt8;
    typedef unsigned char   epicsUInt8;
    typedef short           epicsInt16;
    typedef unsigned short  epicsUInt16;
    typedef epicsUInt16     epicsEnum16;
    typedef int             epicsInt32;
    typedef unsigned        epicsUInt32;
#endif
typedef float           epicsFloat32;
typedef double          epicsFloat64;
typedef unsigned long   epicsIndex;
typedef epicsInt32      epicsStatus;   
#define MAX_STRING_SIZE 40
typedef char            epicsOldString[MAX_STRING_SIZE]; 

/*
 * Imported from Epics header file: ${EPICS_PROJECT_DIR}/base/current/include/epicsTime.h
 */    

typedef struct epicsTimeStamp {
    epicsUInt32    secPastEpoch;   /* seconds since 0000 Jan 1, 1990 */
    epicsUInt32    nsec;           /* nanoseconds within second */
} epicsTimeStamp;

/*
 * Imported from Epics header file: ${EPICS_PROJECT_DIR}/base/current/include/db_access.h
 */    

/* data request buffer types */
#define DBR_STRING      0
#define DBR_SHORT       1
#define DBR_FLOAT       2
#define DBR_ENUM        3
#define DBR_CHAR        4
#define DBR_LONG        5
#define DBR_DOUBLE      6
#define DBR_STS_STRING  7
#define DBR_STS_SHORT   8
#define DBR_STS_FLOAT   9
#define DBR_STS_ENUM    10
#define DBR_STS_CHAR    11
#define DBR_STS_LONG    12
#define DBR_STS_DOUBLE  13
#define DBR_TIME_STRING 14
#define DBR_TIME_INT    15
#define DBR_TIME_SHORT  15
#define DBR_TIME_FLOAT  16
#define DBR_TIME_ENUM   17
#define DBR_TIME_CHAR   18
#define DBR_TIME_LONG   19
#define DBR_TIME_DOUBLE 20
#define DBR_GR_STRING   21
#define DBR_GR_SHORT    22
#define DBR_GR_FLOAT    23
#define DBR_GR_ENUM     24
#define DBR_GR_CHAR     25
#define DBR_GR_LONG     26
#define DBR_GR_DOUBLE   27
#define DBR_CTRL_STRING 28
#define DBR_CTRL_SHORT  29
#define DBR_CTRL_FLOAT  30
#define DBR_CTRL_ENUM   31
#define DBR_CTRL_CHAR   32
#define DBR_CTRL_LONG   33
#define DBR_CTRL_DOUBLE 34

#define dbr_type_is_TIME(type)   \
        ((type) >= DBR_TIME_STRING && (type) <= DBR_TIME_DOUBLE)
        
#define dbr_type_is_CTRL(type)   \
        ((type) >= DBR_CTRL_STRING && (type) <= DBR_CTRL_DOUBLE)

typedef epicsOldString dbr_string_t;
typedef epicsUInt8 dbr_char_t;
typedef epicsInt16 dbr_short_t;
typedef epicsUInt16 dbr_enum_t;
typedef epicsInt32 dbr_long_t;
typedef epicsFloat32 dbr_float_t;
typedef epicsFloat64 dbr_double_t;
typedef epicsUInt16 dbr_put_ackt_t;
typedef epicsUInt16 dbr_put_acks_t;
typedef epicsOldString dbr_stsack_string_t;
typedef epicsOldString dbr_class_name_t;

#define MAX_UNITS_SIZE          8   
#define MAX_ENUM_STRING_SIZE    26
#define MAX_ENUM_STATES         16  

/* structure for a  string time field */
struct dbr_time_string{
    dbr_short_t status;         /* status of value */
    dbr_short_t severity;       /* severity of alarm */
    epicsTimeStamp  stamp;          /* time stamp */
    dbr_string_t    value;          /* current value */
};

/* structure for an short time field */
struct dbr_time_short{
    dbr_short_t status;         /* status of value */
    dbr_short_t severity;       /* severity of alarm */
    epicsTimeStamp  stamp;          /* time stamp */
    dbr_short_t RISC_pad;       /* RISC alignment */
    dbr_short_t value;          /* current value */
};

/* structure for a  float time field */
struct dbr_time_float{
    dbr_short_t status;         /* status of value */
    dbr_short_t severity;       /* severity of alarm */
    epicsTimeStamp  stamp;          /* time stamp */
    dbr_float_t value;          /* current value */
};

/* structure for a  enum time field */
struct dbr_time_enum{
    dbr_short_t status;         /* status of value */
    dbr_short_t severity;       /* severity of alarm */
    epicsTimeStamp  stamp;          /* time stamp */
    dbr_short_t RISC_pad;       /* RISC alignment */
    dbr_enum_t  value;          /* current value */
};

/* structure for a char time field */
struct dbr_time_char{
    dbr_short_t status;         /* status of value */
    dbr_short_t severity;       /* severity of alarm */
    epicsTimeStamp  stamp;          /* time stamp */
    dbr_short_t RISC_pad0;      /* RISC alignment */
    dbr_char_t  RISC_pad1;      /* RISC alignment */
    dbr_char_t  value;          /* current value */
};

/* structure for a long time field */
struct dbr_time_long{
    dbr_short_t status;         /* status of value */
    dbr_short_t severity;       /* severity of alarm */
    epicsTimeStamp  stamp;          /* time stamp */
    dbr_long_t  value;          /* current value */
};

/* structure for a double time field */
struct dbr_time_double{
    dbr_short_t status;         /* status of value */
    dbr_short_t severity;       /* severity of alarm */
    epicsTimeStamp  stamp;          /* time stamp */
    dbr_long_t  RISC_pad;       /* RISC alignment */
    dbr_double_t    value;          /* current value */
};

/* structure for a  string status field */
struct dbr_sts_string {
    dbr_short_t status;         /* status of value */
    dbr_short_t severity;       /* severity of alarm */
    dbr_string_t    value;          /* current value */
};

/* structure for a control integer */
struct dbr_ctrl_int{
    dbr_short_t status;         /* status of value */
    dbr_short_t severity;       /* severity of alarm */
    char        units[MAX_UNITS_SIZE];  /* units of value */
    dbr_short_t upper_disp_limit;   /* upper limit of graph */
    dbr_short_t lower_disp_limit;   /* lower limit of graph */
    dbr_short_t upper_alarm_limit;  
    dbr_short_t upper_warning_limit;
    dbr_short_t lower_warning_limit;
    dbr_short_t lower_alarm_limit;
    dbr_short_t upper_ctrl_limit;   /* upper control limit */
    dbr_short_t lower_ctrl_limit;   /* lower control limit */
    dbr_short_t value;          /* current value */
};
struct dbr_ctrl_short{
    dbr_short_t status;         /* status of value */
    dbr_short_t severity;       /* severity of alarm */
    char        units[MAX_UNITS_SIZE];  /* units of value */
    dbr_short_t upper_disp_limit;   /* upper limit of graph */
    dbr_short_t lower_disp_limit;   /* lower limit of graph */
    dbr_short_t upper_alarm_limit;  
    dbr_short_t upper_warning_limit;
    dbr_short_t lower_warning_limit;
    dbr_short_t lower_alarm_limit;
    dbr_short_t upper_ctrl_limit;   /* upper control limit */
    dbr_short_t lower_ctrl_limit;   /* lower control limit */
    dbr_short_t value;          /* current value */
};

/* structure for a control floating point field */
struct dbr_ctrl_float{
    dbr_short_t status;         /* status of value */
    dbr_short_t severity;       /* severity of alarm */
    dbr_short_t precision;      /* number of decimal places */
    dbr_short_t RISC_pad;       /* RISC alignment */
    char        units[MAX_UNITS_SIZE];  /* units of value */
    dbr_float_t upper_disp_limit;   /* upper limit of graph */
    dbr_float_t lower_disp_limit;   /* lower limit of graph */
    dbr_float_t upper_alarm_limit;  
    dbr_float_t upper_warning_limit;
    dbr_float_t lower_warning_limit;
    dbr_float_t lower_alarm_limit;
    dbr_float_t upper_ctrl_limit;   /* upper control limit */
    dbr_float_t lower_ctrl_limit;   /* lower control limit */
    dbr_float_t value;          /* current value */
};

/* structure for a control enumeration field */
struct dbr_ctrl_enum{
    dbr_short_t status;         /* status of value */
    dbr_short_t severity;       /* severity of alarm */
    dbr_short_t no_str;         /* number of strings */
    char    strs[MAX_ENUM_STATES][MAX_ENUM_STRING_SIZE];
                    /* state strings */
    dbr_enum_t  value;      /* current value */
};

/* structure for a control char field */
struct dbr_ctrl_char{
    dbr_short_t status;         /* status of value */
    dbr_short_t severity;       /* severity of alarm */
    char        units[MAX_UNITS_SIZE];  /* units of value */
    dbr_char_t  upper_disp_limit;   /* upper limit of graph */
    dbr_char_t  lower_disp_limit;   /* lower limit of graph */
    dbr_char_t  upper_alarm_limit;  
    dbr_char_t  upper_warning_limit;
    dbr_char_t  lower_warning_limit;
    dbr_char_t  lower_alarm_limit;
    dbr_char_t  upper_ctrl_limit;   /* upper control limit */
    dbr_char_t  lower_ctrl_limit;   /* lower control limit */
    dbr_char_t  RISC_pad;       /* RISC alignment */
    dbr_char_t  value;          /* current value */
};

/* structure for a control long field */
struct dbr_ctrl_long{
    dbr_short_t status;         /* status of value */
    dbr_short_t severity;       /* severity of alarm */
    char        units[MAX_UNITS_SIZE];  /* units of value */
    dbr_long_t  upper_disp_limit;   /* upper limit of graph */
    dbr_long_t  lower_disp_limit;   /* lower limit of graph */
    dbr_long_t  upper_alarm_limit;  
    dbr_long_t  upper_warning_limit;
    dbr_long_t  lower_warning_limit;
    dbr_long_t  lower_alarm_limit;
    dbr_long_t  upper_ctrl_limit;   /* upper control limit */
    dbr_long_t  lower_ctrl_limit;   /* lower control limit */
    dbr_long_t  value;          /* current value */
};

/* structure for a control double field */
struct dbr_ctrl_double{
    dbr_short_t status;         /* status of value */
    dbr_short_t severity;       /* severity of alarm */
    dbr_short_t precision;      /* number of decimal places */
    dbr_short_t RISC_pad0;      /* RISC alignment */
    char        units[MAX_UNITS_SIZE];  /* units of value */
    dbr_double_t    upper_disp_limit;   /* upper limit of graph */
    dbr_double_t    lower_disp_limit;   /* lower limit of graph */
    dbr_double_t    upper_alarm_limit;  
    dbr_double_t    upper_warning_limit;
    dbr_double_t    lower_warning_limit;
    dbr_double_t    lower_alarm_limit;
    dbr_double_t    upper_ctrl_limit;   /* upper control limit */
    dbr_double_t    lower_ctrl_limit;   /* lower control limit */
    dbr_double_t    value;          /* current value */
};

#endif 
// #ifndef EPICS_HEADERS_INCLUDED    

/*
 * Imported from Epics header file: ${EPICS_PROJECT_DIR}/base/current/include/alarm.h
 */    
extern const char *epicsAlarmSeverityStrings [ALARM_NSEV];
extern const char *epicsAlarmConditionStrings [ALARM_NSTATUS];

/*
 * Imported from Epics source file: ${EPICS_PROJECT_DIR}/base/current/src/ca/access.cpp
 *  
 */
extern const char *dbr_text[35];

} // namespace Epics
    
        
namespace EpicsDbrTools
{
    
const int iSizeBasicDbrTypes  = 7;  // Summarized from Epics namespace
const int iSizeAllDbrTypes    = 35; // Summarized from Epics namespace

/**
 *  Maximum PV Name Length: 
 * 
 *  Manually defined hered. By convention, formal Epics PV names are usually less than 40 characters, 
 *  so here we use a big enough value as the buffer size.
 *  Note that this value corresponds to some epics data structure size in existing xtc files.
 **/
const int iMaxPvNameLength    = 64; 

using namespace Epics;
/*
 * CaDbr Tool classes
 */
template <int iDbrType> struct DbrTypeFromInt { char a[-1 + iDbrType*0]; }; // Default should not be used. Will generate compilation error.
template <> struct DbrTypeFromInt<DBR_STRING> { typedef dbr_string_t TDbr; };
template <> struct DbrTypeFromInt<DBR_SHORT>  { typedef dbr_short_t  TDbr; };
template <> struct DbrTypeFromInt<DBR_FLOAT>  { typedef dbr_float_t  TDbr; };
template <> struct DbrTypeFromInt<DBR_ENUM>   { typedef dbr_enum_t   TDbr; };
template <> struct DbrTypeFromInt<DBR_CHAR>   { typedef dbr_char_t   TDbr; };
template <> struct DbrTypeFromInt<DBR_LONG>   { typedef dbr_long_t   TDbr; };
template <> struct DbrTypeFromInt<DBR_DOUBLE> { typedef dbr_double_t TDbr; };

template <> struct DbrTypeFromInt<DBR_TIME_STRING> { typedef dbr_time_string TDbr; };
template <> struct DbrTypeFromInt<DBR_TIME_SHORT>  { typedef dbr_time_short  TDbr; };
template <> struct DbrTypeFromInt<DBR_TIME_FLOAT>  { typedef dbr_time_float  TDbr; };
template <> struct DbrTypeFromInt<DBR_TIME_ENUM>   { typedef dbr_time_enum   TDbr; };
template <> struct DbrTypeFromInt<DBR_TIME_CHAR>   { typedef dbr_time_char   TDbr; };
template <> struct DbrTypeFromInt<DBR_TIME_LONG>   { typedef dbr_time_long   TDbr; };
template <> struct DbrTypeFromInt<DBR_TIME_DOUBLE> { typedef dbr_time_double TDbr; };

template <> struct DbrTypeFromInt<DBR_CTRL_STRING> { typedef dbr_sts_string  TDbr; };
template <> struct DbrTypeFromInt<DBR_CTRL_SHORT>  { typedef dbr_ctrl_short  TDbr; };
template <> struct DbrTypeFromInt<DBR_CTRL_FLOAT>  { typedef dbr_ctrl_float  TDbr; };
template <> struct DbrTypeFromInt<DBR_CTRL_ENUM>   { typedef dbr_ctrl_enum   TDbr; };
template <> struct DbrTypeFromInt<DBR_CTRL_CHAR>   { typedef dbr_ctrl_char   TDbr; };
template <> struct DbrTypeFromInt<DBR_CTRL_LONG>   { typedef dbr_ctrl_long   TDbr; };
template <> struct DbrTypeFromInt<DBR_CTRL_DOUBLE> { typedef dbr_ctrl_double TDbr; };

template <int iDbrType> struct DbrTypeTraits
{ 
    enum { 
      iDiffDbrOrgToDbrTime = DBR_TIME_DOUBLE - DBR_DOUBLE,
      iDiffDbrOrgToDbrCtrl = DBR_CTRL_DOUBLE - DBR_DOUBLE };
      
    enum { 
      iDbrTimeType = iDbrType + iDiffDbrOrgToDbrTime,
      iDbrCtrlType = iDbrType + iDiffDbrOrgToDbrCtrl };
        
    typedef typename DbrTypeFromInt<iDbrType    >::TDbr TDbrOrg;
    typedef typename DbrTypeFromInt<iDbrTimeType>::TDbr TDbrTime;
    typedef typename DbrTypeFromInt<iDbrCtrlType>::TDbr TDbrCtrl;    
}; 

template <class TDbr> struct DbrIntFromType { char a[-1 + (TDbr)0]; }; // Default should not be used. Will generate compilation error.
template <class TDbr> struct DbrIntFromType<const TDbr> : DbrIntFromType<TDbr> {}; // remove const qualifier
template <> struct DbrIntFromType<dbr_string_t> { enum {iTypeId = DBR_STRING }; };
template <> struct DbrIntFromType<dbr_short_t>  { enum {iTypeId = DBR_SHORT  }; };
template <> struct DbrIntFromType<dbr_float_t>  { enum {iTypeId = DBR_FLOAT  }; };
template <> struct DbrIntFromType<dbr_enum_t>   { enum {iTypeId = DBR_ENUM   }; };
template <> struct DbrIntFromType<dbr_char_t>   { enum {iTypeId = DBR_CHAR   }; };
template <> struct DbrIntFromType<dbr_long_t>   { enum {iTypeId = DBR_LONG   }; };
template <> struct DbrIntFromType<dbr_double_t> { enum {iTypeId = DBR_DOUBLE }; };

template <class TDbr> struct DbrIntFromCtrlType { char a[-1 + (TDbr)0]; }; // Default should not be used. Will generate compilation error.
template <class TDbr> struct DbrIntFromCtrlType<const TDbr> : DbrIntFromCtrlType<TDbr> {}; // remove const qualifier
template <> struct DbrIntFromCtrlType<dbr_sts_string>  { enum {iTypeId = DBR_STRING }; };
template <> struct DbrIntFromCtrlType<dbr_ctrl_short>  { enum {iTypeId = DBR_SHORT  }; };
template <> struct DbrIntFromCtrlType<dbr_ctrl_float>  { enum {iTypeId = DBR_FLOAT  }; };
template <> struct DbrIntFromCtrlType<dbr_ctrl_enum>   { enum {iTypeId = DBR_ENUM   }; };
template <> struct DbrIntFromCtrlType<dbr_ctrl_char>   { enum {iTypeId = DBR_CHAR   }; };
template <> struct DbrIntFromCtrlType<dbr_ctrl_long>   { enum {iTypeId = DBR_LONG   }; };
template <> struct DbrIntFromCtrlType<dbr_ctrl_double> { enum {iTypeId = DBR_DOUBLE }; };

extern const char* sDbrPrintfFormat[];

template <class TDbr> 
void printValue( TDbr* pValue ) 
{ 
    enum { iDbrType = DbrIntFromType<TDbr>::iTypeId };
    printf( sDbrPrintfFormat[iDbrType], *pValue );    
}

template <> inline
void printValue( const dbr_string_t* pValue )
{ 
    printf( "%s", (char*) pValue );
}

template <class TCtrl> // Default not to print precision field
void printPrecisionField(TCtrl& pvCtrlVal)  {}

template <> inline 
void printPrecisionField(const dbr_ctrl_double& pvCtrlVal)
{
    printf( "Precision: %d\n", pvCtrlVal.precision );
}

template <> inline 
void printPrecisionField(const dbr_ctrl_float& pvCtrlVal)
{
    printf( "Precision: %d\n", pvCtrlVal.precision );    
}

template <class TCtrl> inline
void printCtrlFields(TCtrl& pvCtrlVal)
{
    printPrecisionField(pvCtrlVal);
    printf( "Units: %s\n", pvCtrlVal.units );        
    
    enum { iDbrType = DbrIntFromCtrlType<TCtrl>::iTypeId };    

    std::string sFieldFmt = sDbrPrintfFormat[iDbrType];
    std::string sOutputString = std::string() +
      "Hi Disp : " + sFieldFmt + "  Lo Disp : " + sFieldFmt + "\n" + 
      "Hi Alarm: " + sFieldFmt + "  Hi Warn : " + sFieldFmt + "\n" +
      "Lo Warn : " + sFieldFmt + "  Lo Alarm: " + sFieldFmt + "\n" +
      "Hi Ctrl : " + sFieldFmt + "  Lo Ctrl : " + sFieldFmt + "\n";
     
    printf( sOutputString.c_str(), 
      pvCtrlVal.upper_disp_limit, pvCtrlVal.lower_disp_limit, 
      pvCtrlVal.upper_alarm_limit, pvCtrlVal.upper_warning_limit, 
      pvCtrlVal.lower_warning_limit, pvCtrlVal.lower_alarm_limit, 
      pvCtrlVal.upper_ctrl_limit, pvCtrlVal.lower_ctrl_limit
      );
    
    return;
}

template <> inline 
void printCtrlFields(const dbr_sts_string& pvCtrlVal) {}

template <> inline 
void printCtrlFields(const dbr_ctrl_enum& pvCtrlVal) 
{
    if ( pvCtrlVal.no_str > 0 )
    {
        printf( "EnumState Num: %d\n", pvCtrlVal.no_str );
        
        for (int iEnumState = 0; iEnumState < pvCtrlVal.no_str; iEnumState++ )
            printf( "EnumState[%d]: %s\n", iEnumState, pvCtrlVal.strs[iEnumState] );
    }
}

} // namespace EpicsDbrTools

} // namespace Pds

#endif
